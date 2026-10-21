#include "AetherHealingFieldActor.h"

#include "AbilitySystemComponent.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AttributeSet/AetherBaseAttributeSet.h"
#include "Aether/AbilitySystem/Effects/AetherGameplayEffect_HealingField.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Aether/PartySystem/AetherPartyComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AAetherHealingFieldActor::AAetherHealingFieldActor()
{
	PrimaryActorTick.bCanEverTick = false;
	FieldVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FieldVisual"));
	SetRootComponent(FieldVisual);
	FieldVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FieldVisual->SetGenerateOverlapEvents(false);
	FieldVisual->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	FieldVisual->SetStaticMesh(Mesh.Object);
	SetActorHiddenInGame(true);
}

void AAetherHealingFieldActor::StartField(UAbilitySystemComponent* InCaster, UAetherPartyComponent* InParty, float InHealingPerTick)
{
	if (bStarted || !IsValid(InCaster) || !GetWorld())
		return;
	bStarted = true;
	CasterASC = InCaster;
	// Bind before the immediate heal: its callbacks can also kill the caster.
	CasterHealthChangedHandle = InCaster->GetGameplayAttributeValueChangeDelegate(
		UAetherBaseAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::OnCasterHealthChanged);
	if (InCaster->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute()) <= 0.f)
	{
		Destroy();
		return;
	}
	SourceParty = InParty;
	bHasSourceParty = InParty != nullptr;
	HealingPerTick = FMath::FloorToFloat(FMath::Max(0.f, InHealingPerTick));
	FieldVisual->SetRelativeScale3D(FVector(Radius / 50.f, Radius / 50.f, 0.02f));
	if (FieldMaterial)
		FieldVisual->SetMaterial(0, FieldMaterial);
	SetActorHiddenInGame(false);
	GetWorldTimerManager().SetTimer(ExpirationTimer, this, &ThisClass::Expire, Duration, false);
	HealAndScheduleNext();
}

void AAetherHealingFieldActor::HealAndScheduleNext()
{
	// Use the same clock as expiration. A delayed callback must never heal after expiry.
	const float Remaining = GetWorldTimerManager().GetTimerRemaining(ExpirationTimer);
	if (IsActorBeingDestroyed() || Remaining <= 0.f)
		return;
	const float Elapsed = Duration - Remaining;
	HealCurrentTarget();
	if (IsActorBeingDestroyed())
		return;
	// One callback per frame at most; skip missed slots and keep the original phase.
	NextHealingSlot = FMath::Max(NextHealingSlot + 1, FMath::FloorToInt(Elapsed) + 1);
	const float NextTime = static_cast<float>(NextHealingSlot);
	if (NextTime < Duration)
		GetWorldTimerManager().SetTimer(HealingTimer, this,
			&ThisClass::HealAndScheduleNext, NextTime - Elapsed, false);
}

void AAetherHealingFieldActor::HealCurrentTarget()
{
	UAbilitySystemComponent* Source = CasterASC.Get();
	UAetherPartyComponent* Party = SourceParty.Get();
	if (!Source || (bHasSourceParty && !Party))
		return;
	UAbilitySystemComponent* Target = Party ? Party->GetActiveAbilitySystemComponent() : Source;
	ACharacter* Character = Target ? Cast<ACharacter>(Target->GetAvatarActor()) : nullptr;
	if (!Character || HealingPerTick <= 0.f)
		return;
	const float Health = Target->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute());
	if (Health <= 0.f)
		return;
	const FVector Feet = Character->GetActorLocation()
		- FVector(0, 0, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	// Visual is raised 2cm from the sampled ground; use ground as the range origin.
	const FVector Center = GetActorLocation() - GetActorUpVector() * 2.f;
	if (FVector::DistSquared2D(Feet, Center) > FMath::Square(Radius)
		|| FMath::Abs(Feet.Z - Center.Z) > HeightTolerance)
		return;
	const float Missing = FMath::Max(0.f,
		Target->GetNumericAttribute(UAetherBaseAttributeSet::GetMaxHealthAttribute()) - Health);
	const float FieldAmount = FMath::FloorToFloat(FMath::Min(HealingPerTick, Missing));
	const float Overflow = FMath::FloorToFloat(FMath::Max(0.f, HealingPerTick - Missing));
	// Snapshot recipients and amounts before applying any GE (attribute callbacks can mutate state).
	struct FPendingHeal
	{
		TWeakObjectPtr<UAbilitySystemComponent> ASC;
		float Amount;
	};
	TArray<FPendingHeal> Pending;
	if (Party && Overflow > 0.f)
	{
		const auto Standby = Party->GetLivingStandbyAbilitySystemComponents();
		const float Share = Standby.IsEmpty() ? 0.f : FMath::FloorToFloat(Overflow / Standby.Num());
		for (UAbilitySystemComponent* Recipient : Standby)
		{
			const float Deficit = FMath::Max(0.f,
				Recipient->GetNumericAttribute(UAetherBaseAttributeSet::GetMaxHealthAttribute())
				- Recipient->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute()));
			Pending.Add({Recipient, FMath::FloorToFloat(FMath::Min(Share, Deficit))});
		}
	}
	ApplyHealing(Target, FieldAmount);
	for (const FPendingHeal& Heal : Pending)
		ApplyHealing(Heal.ASC.Get(), Heal.Amount);
}

void AAetherHealingFieldActor::ApplyHealing(UAbilitySystemComponent* Target, float RequestedAmount)
{
	UAbilitySystemComponent* Source = CasterASC.Get();
	if (IsActorBeingDestroyed() || !IsValid(Source) || !IsValid(Target) || RequestedAmount <= 0.f)
		return;
	const float Health = Target->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute());
	if (Health <= 0.f)
		return;
	// Recheck cap and survival, but never redistribute a lost share.
	const float Missing = FMath::Max(0.f,
		Target->GetNumericAttribute(UAetherBaseAttributeSet::GetMaxHealthAttribute()) - Health);
	const float Amount = FMath::FloorToFloat(FMath::Min(RequestedAmount, Missing));
	if (Amount <= 0.f)
		return;
	FGameplayEffectContextHandle Context = Source->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle Spec = Source->MakeOutgoingSpec(
		UAetherGameplayEffect_HealingField::StaticClass(), 1.f, Context);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(UAetherGameplayEffect_HealingField::AmountName, Amount);
		Source->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), Target);
	}
}

void AAetherHealingFieldActor::OnCasterHealthChanged(const FOnAttributeChangeData& Change)
{
	if (Change.NewValue <= 0.f)
		Destroy();
}

void AAetherHealingFieldActor::Expire()
{
	Destroy();
}

void AAetherHealingFieldActor::EndPlay(const EEndPlayReason::Type Reason)
{
	GetWorldTimerManager().ClearTimer(ExpirationTimer);
	GetWorldTimerManager().ClearTimer(HealingTimer);
	if (UAbilitySystemComponent* Source = CasterASC.Get())
		Source->GetGameplayAttributeValueChangeDelegate(UAetherBaseAttributeSet::GetHealthAttribute())
			.Remove(CasterHealthChangedHandle);
	CasterHealthChangedHandle.Reset();
	CasterASC.Reset();
	SourceParty.Reset();
	Super::EndPlay(Reason);
}
