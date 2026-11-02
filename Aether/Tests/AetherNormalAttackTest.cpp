#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/ActiveMontageInstanceScope.h"
#include "Animation/AnimNotifyQueue.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/WorldSettings.h"
#include "Aether/AetherGameplayTags.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AetherAbilitySet.h"
#include "Aether/AbilitySystem/AetherCharacterDefinition.h"
#include "Aether/AbilitySystem/Abilities/AetherGameplayAbility_NormalAttack.h"
#include "Aether/AbilitySystem/AttributeSet/AetherBaseAttributeSet.h"
#include "Aether/AbilitySystem/AttributeSet/AetherHeroAttributeSet.h"
#include "Aether/Animation/AnimNotify/AnimNotifyState_AttackTrace.h"
#include "Aether/PartySystem/AetherFieldCharacterPawn.h"
#include "Aether/PartySystem/AetherPartyMemberStateActor.h"
#include "Aether/Debug/AetherAttackTestTarget.h"

struct FNormalAttackTestAccess
{
	static void Configure(UAetherGameplayAbility_NormalAttack *A, UAnimMontage *M)
	{
		A->GroundAttackAnimations = {M};
	}
	static int32 Instance(UAetherGameplayAbility_NormalAttack *A)
	{
		return A->MontageInstanceId;
	}
	static int32 Windows(UAetherGameplayAbility_NormalAttack *A)
	{
		return A->TraceSessions.Num();
	}
	static void Damage(UAetherGameplayAbility_NormalAttack *A, float Value)
	{
		A->BaseDamage = Value;
	}
};

namespace NormalAttackTest
{
struct FFixture
{
	UWorld *World;
	AAetherFieldCharacterPawn *Pawn;
	AAetherPartyMemberStateActor *Owner;
	UAetherAbilitySystemComponent *ASC;
	UAetherGameplayAbility_NormalAttack *Ability = nullptr;
	UAnimMontage *Montage;
	FGameplayAbilitySpecHandle Handle;
	FFixture()
	{
		const auto Values = UWorld::InitializationValues()
		                        .AllowAudioPlayback(false)
		                        .CreatePhysicsScene(true)
		                        .CreateNavigation(false)
		                        .CreateAISystem(false)
		                        .ShouldSimulatePhysics(false)
		                        .SetTransactional(false);
		World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
		GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		Pawn = World->SpawnActor<AAetherFieldCharacterPawn>();
		World->SpawnActor<AAIController>()->Possess(Pawn);
		auto *SourceDefinition = LoadObject<UAetherCharacterDefinition>(nullptr, TEXT("/Game/Aether/Characters/DA_Yuito.DA_Yuito"));
		if (SourceDefinition)
			Pawn->ApplyCharacterAppearance(SourceDefinition->DisplayMesh, SourceDefinition->DisplayAnimInstanceClass);
		Pawn->GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		Pawn->GetCharacterMovement()->DisableMovement();
		auto *Definition = NewObject<UAetherCharacterDefinition>(World);
		auto *Set = NewObject<UAetherAbilitySet>(World);
		FAetherAbilitySet_GameplayAbility Entry;
		Entry.Ability = UAetherGameplayAbility_NormalAttack::StaticClass();
		Entry.InputTag = AetherGameplayTags::InputTag_NormalAttack;
		Set->Abilities.Add(Entry);
		Definition->AbilitySets.Add(Set);
		Owner = World->SpawnActor<AAetherPartyMemberStateActor>();
		Owner->InitializeCharacterRuntime(Definition, Pawn);
		ASC = Owner->GetAetherAbilitySystemComponent();
		ASC->AddAttributeSetSubobject(NewObject<UAetherHeroAttributeSet>(Owner));
		World->BeginPlay();
		// This isolated world has no GameMode to dispatch actor BeginPlay.
		World->GetWorldSettings()->NotifyBeginPlay();
		Handle = ASC->GetActivatableAbilities()[0].Handle;
		Ability = Cast<UAetherGameplayAbility_NormalAttack>(ASC->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());
		Montage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/Aether/Ability/NormalAttackStage1/AM_SingleAttack.AM_SingleAttack"));
		if (Ability)
			FNormalAttackTestAccess::Configure(Ability, Montage);
		Advance(0.05f);
	}
	~FFixture()
	{
		ASC->CancelAllAbilities();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
	void Advance(float Seconds)
	{
		for (int32 I = 0; I < FMath::CeilToInt(Seconds / 0.02f); ++I)
		{
			++GFrameCounter;
			World->Tick(LEVELTICK_All, 0.02f);
		}
	}
	bool Press()
	{
		ASC->AbilityInputPressed(AetherGameplayTags::InputTag_NormalAttack);
		ASC->ProcessInputs();
		ASC->AbilityInputReleased(AetherGameplayTags::InputTag_NormalAttack);
		ASC->ProcessInputs();
		return Ability && Ability->IsActive();
	}
	AAetherAttackTestTarget *Target(FVector Location)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World->SpawnActor<AAetherAttackTestTarget>(Location, FRotator::ZeroRotator, Params);
	}
};
} // namespace NormalAttackTest

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherNormalAttackWindowsTest, "Aether.NormalAttack.WindowsAndDamage",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherNormalAttackWindowsTest::RunTest(const FString &Parameters)
{
	NormalAttackTest::FFixture F;
	if (!TestNotNull(TEXT("Configured montage exists"), F.Montage) || !TestTrue(TEXT("Input activates attack"), F.Press()))
		return false;
	const FVector Hand = F.Pawn->GetMesh()->GetSocketLocation(TEXT("hand_r"));
	const int32 OriginalInstance = FNormalAttackTestAccess::Instance(F.Ability);
	F.Press();
	TestEqual(TEXT("Extra click does not restart montage"), FNormalAttackTestAccess::Instance(F.Ability), OriginalInstance);
	auto *A = F.Target(Hand + FVector(0, 15, 0));
	auto *B = F.Target(Hand + FVector(0, -15, 0));
	auto *Notify = NewObject<UAnimNotifyState_AttackTrace>();
	Notify->Settings.LocalEnd = FVector::ZeroVector;
	Notify->Settings.Radius = 40.f;
	FAnimNotifyEvent Window;
	FAnimNotifyEventReference Event(&Window, F.Montage);
	Event.AddContextData<UE::Anim::FAnimNotifyMontageInstanceContext>(FNormalAttackTestAccess::Instance(F.Ability));
	Notify->NotifyTick(F.Pawn->GetMesh(), F.Montage, .02f, Event);
	TestEqual(TEXT("Tick before Begin gives no damage"), A->Attributes->GetHealth(), 1000.f);
	Notify->NotifyBegin(F.Pawn->GetMesh(), F.Montage, .2f, Event);
	TestEqual(TEXT("Base 100 -> physical damage 50 to target A"), A->Attributes->GetHealth(), 950.f);
	TestEqual(TEXT("Second blocking target also hit"), B->Attributes->GetHealth(), 950.f);
	for (int32 I = 0; I < 4; ++I)
		Notify->NotifyTick(F.Pawn->GetMesh(), F.Montage, .02f, Event);
	TestEqual(TEXT("Repeated samples do not duplicate damage"), A->Attributes->GetHealth(), 950.f);
	Notify->NotifyEnd(F.Pawn->GetMesh(), F.Montage, Event);
	Notify->NotifyTick(F.Pawn->GetMesh(), F.Montage, .02f, Event);
	TestEqual(TEXT("Late Tick does not recreate state"), FNormalAttackTestAccess::Windows(F.Ability), 0);
	FAnimNotifyEvent Window2;
	FAnimNotifyEventReference Event2(&Window2, F.Montage);
	Event2.AddContextData<UE::Anim::FAnimNotifyMontageInstanceContext>(FNormalAttackTestAccess::Instance(F.Ability));
	FNormalAttackTestAccess::Damage(F.Ability, 200.f);
	Notify->NotifyBegin(F.Pawn->GetMesh(), F.Montage, .2f, Event2);
	TestEqual(TEXT("Next window with base 200 deals 100"), A->Attributes->GetHealth(), 850.f);
	Notify->NotifyEnd(F.Pawn->GetMesh(), F.Montage, Event);
	TestEqual(TEXT("Old End cannot close new window"), FNormalAttackTestAccess::Windows(F.Ability), 1);
	F.ASC->CancelAbilityHandle(F.Handle);
	TestEqual(TEXT("Cancel clears windows"), FNormalAttackTestAccess::Windows(F.Ability), 0);
	Notify->NotifyBegin(F.Pawn->GetMesh(), F.Montage, .2f, Event);
	TestEqual(TEXT("Canceled Begin does not hit"), A->Attributes->GetHealth(), 850.f);
	TestTrue(TEXT("Fresh attack can activate"), F.Press());
	Notify->NotifyBegin(F.Pawn->GetMesh(), F.Montage, .2f, Event);
	TestEqual(TEXT("Previous montage instance cannot start a window"), FNormalAttackTestAccess::Windows(F.Ability), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherNormalAttackPlaybackTest, "Aether.NormalAttack.RealMontageLifecycle",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherNormalAttackPlaybackTest::RunTest(const FString &Parameters)
{
	NormalAttackTest::FFixture F;
	if (!TestNotNull(TEXT("Content montage"), F.Montage) || !TestTrue(TEXT("Press starts montage"), F.Press()))
		return false;
	// A wide target covers the hand trajectory; this tests actual animation Notify dispatch.
	auto *Target = F.Target(F.Pawn->GetActorLocation());
	Target->Collision->SetCapsuleSize(200.f, 200.f);
	F.Advance(0.15f);
	TestEqual(TEXT("Before authored window no damage"), Target->Attributes->GetHealth(), 1000.f);
	F.Advance(0.4f);
	TestEqual(TEXT("Real Notify context reaches registered ability once"), Target->Attributes->GetHealth(), 950.f);
	F.Advance(1.f);
	TestFalse(TEXT("Montage completion ends ability"), F.Ability->IsActive());
	TestEqual(TEXT("Completed attack leaves no windows"), FNormalAttackTestAccess::Windows(F.Ability), 0);
	TestTrue(TEXT("Next click reactivates"), F.Press());
	F.ASC->CancelAbilityHandle(F.Handle);
	F.Advance(1.f);
	TestEqual(TEXT("Cancellation before window prevents later hit"), Target->Attributes->GetHealth(), 950.f);
	FNormalAttackTestAccess::Configure(F.Ability, nullptr);
	TestFalse(TEXT("Missing montage fails without leaving ability active"), F.Press());
	FNormalAttackTestAccess::Configure(F.Ability, F.Montage);
	TestTrue(TEXT("Valid attack can start after failed activation"), F.Press());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherNormalAttackGeometryTest, "Aether.NormalAttack.Geometry",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherNormalAttackGeometryTest::RunTest(const FString &Parameters)
{
	NormalAttackTest::FFixture F;
	FAetherMeleeTraceSettings Settings;
	FVector A, B, C, D;
	TestTrue(TEXT("Character socket resolves without weapon"), AetherMeleeTrace::ResolveSegment(F.Pawn->GetMesh(), Settings, A, B));
	Settings.ReferenceSocket = TEXT("MissingSocket");
	TestFalse(TEXT("Missing socket is rejected"), AetherMeleeTrace::ResolveSegment(F.Pawn->GetMesh(), Settings, C, D));
	Settings.ReferenceSocket = TEXT("hand_r");
	// An unrelated weapon visual cannot affect the segment; only the character socket is an input.
	auto *Visual = NewObject<UStaticMeshComponent>(F.Pawn);
	Visual->RegisterComponent();
	Visual->AttachToComponent(F.Pawn->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("hand_r"));
	Visual->SetRelativeScale3D(FVector(10.f));
	Visual->SetRelativeLocation(FVector(500.f));
	AetherMeleeTrace::ResolveSegment(F.Pawn->GetMesh(), Settings, C, D);
	TestTrue(TEXT("Weapon visual transform does not change start/end"), A.Equals(C) && B.Equals(D));
	return true;
}
#endif
