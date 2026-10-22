#include "AetherGameplayAbility_HealingField.h"
#include "Aether/Aether.h"

#include "Aether/AbilitySystem/Actors/AetherHealingFieldActor.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AttributeSet/AetherBaseAttributeSet.h"
#include "Aether/PartySystem/AetherPartyComponent.h"
#include "Aether/PartySystem/AetherPartyMemberStateActor.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UAetherGameplayAbility_HealingField::UAetherGameplayAbility_HealingField()
{
	FieldClass = AAetherHealingFieldActor::StaticClass();
}

void UAetherGameplayAbility_HealingField::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	auto Finish = [&](bool bCancelled)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, bCancelled);
	};
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	AAetherPartyMemberStateActor* Member = ActorInfo ? Cast<AAetherPartyMemberStateActor>(ActorInfo->OwnerActor.Get()) : nullptr;
	ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UAetherPartyComponent* Party = Member ? Member->GetPartyComponent() : nullptr;
	if (!ASC || !Member || !Character || !FieldClass || ASC->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute()) <= 0.f || !Character->GetCharacterMovement()->IsMovingOnGround() || (Party && Party->GetActiveAbilitySystemComponent() != ASC) || !CommitCheck(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogAether, Verbose, TEXT("HealingField rejected: ASC=%s Member=%s Character=%s Health=%g Grounded=%d"), *GetNameSafe(ASC), *GetNameSafe(Member), *GetNameSafe(Character), ASC ? ASC->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute()) : -1.f, Character && Character->GetCharacterMovement()->IsMovingOnGround());
		Finish(true);
		return;
	}

	// Snapshot before spawning or committing effects can change attributes.
	const float HealingPerTick = FMath::FloorToFloat(FMath::Max(0.f, ASC->GetNumericAttribute(UAetherBaseAttributeSet::GetMaxHealthAttribute())) * FMath::Clamp(HealingPercent, 0.f, 100.f) / 100.f);
	const FVector Feet = Character->GetActorLocation() - FVector(0, 0, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FCollisionQueryParams Query(SCENE_QUERY_STAT(HealingFieldGround), false, Character);
	FHitResult Ground;
	if (!GetWorld()->LineTraceSingleByChannel(Ground, Feet + FVector(0, 0, 20), Feet - FVector(0, 0, 100), ECC_Visibility, Query) || !Character->GetCharacterMovement()->IsWalkable(Ground))
	{
		UE_LOG(LogAether, Verbose, TEXT("HealingField rejected ground: Feet=%s Hit=%d Normal=%s"), *Feet.ToString(), Ground.bBlockingHit, *Ground.ImpactNormal.ToString());
		Finish(true);
		return;
	}

	const FTransform Transform(FRotationMatrix::MakeFromZ(Ground.ImpactNormal).ToQuat(), Ground.ImpactPoint);
	AAetherHealingFieldActor* NewField = GetWorld()->SpawnActorDeferred<AAetherHealingFieldActor>(FieldClass, Transform, Member, Character, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!NewField)
	{
		Finish(true);
		return;
	}
	NewField->FinishSpawning(Transform);
	if (!IsValid(NewField) || !CommitAbility(Handle, ActorInfo, ActivationInfo) || !IsValid(Member) || !IsValid(Character) || ASC->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute()) <= 0.f || (Party && Party->GetActiveAbilitySystemComponent() != ASC))
	{
		if (IsValid(NewField))
		{
			NewField->Destroy();
		}
		Finish(true);
		return;
	}

	AAetherHealingFieldActor* Previous = SpawnedField.Get();
	SpawnedField = NewField;
	if (IsValid(Previous))
	{
		Previous->Destroy();
	}
	NewField->StartField(ASC, Party, HealingPerTick);
	Finish(false);
}
