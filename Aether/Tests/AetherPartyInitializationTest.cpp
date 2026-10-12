#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Aether/AetherGameMode.h"
#include "Aether/Player/AetherPlayerState.h"
#include "Aether/Player/AetherPlayerController.h"
#include "Aether/PartySystem/AetherFieldCharacterPawn.h"
#include "Aether/PartySystem/AetherPartyMemberStateActor.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AetherCharacterDefinition.h"
#include "Aether/AbilitySystem/Effects/AetherGameplayEffect_SwitchCooldown.h"
#include "Aether/AbilitySystem/Abilities/AetherGameplayAbility_Jump.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/GarbageCollection.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherPartyInitializationTest,
	"Aether.Party.InitializationAndPawnAttachment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAetherPartyInitializationTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Native GameMode uses the shared field pawn"),
		GetDefault<AAetherGameMode>()->DefaultPawnClass.Get(), AAetherFieldCharacterPawn::StaticClass());

	for (bool bPawnFirst : {false, true})
	{
		const UWorld::InitializationValues WorldValues = UWorld::InitializationValues()
			.AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false)
			.CreateAISystem(false).ShouldSimulatePhysics(false).SetTransactional(false);
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr,
			true, ERHIFeatureLevel::Num, &WorldValues);

		AAetherPlayerState* PS = World->SpawnActor<AAetherPlayerState>();
		AAetherPlayerController* Controller = World->SpawnActor<AAetherPlayerController>();
		Controller->PlayerState = PS;
		PS->SetOwner(Controller);
		UAetherPartyComponent* Party = PS->GetPartyComponent();
		TestNotNull(TEXT("Initial loading callback remains reflected"), Party->FindFunction(TEXT("HandleInitialCharacterLoaded")));
		UAetherAbilitySystemComponent* PartyASC = PS->GetPartyASC();
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AAetherFieldCharacterPawn* Pawn = World->SpawnActor<AAetherFieldCharacterPawn>(
			AAetherFieldCharacterPawn::StaticClass(), FTransform::Identity, SpawnParams);
		UAetherCharacterDefinition* Definition = NewObject<UAetherCharacterDefinition>(
			World, bPawnFirst ? TEXT("PawnFirstDefinition") : TEXT("PartyFirstDefinition"));
		const FPrimaryAssetId Id = Definition->GetPrimaryAssetId();

		if (bPawnFirst)
		{
			Controller->Possess(Pawn);
			TestEqual(TEXT("Possession attaches field pawn"), Party->GetFieldCharacterPawn(), Pawn);
			TestEqual(TEXT("Attachment alone does not grant abilities"), PartyASC->GetActivatableAbilities().Num(), 0);
		}
		Party->InitializeParty({Id});
		Party->HandleInitialCharacterLoaded(Id, Definition);
		if (!bPawnFirst)
		{
			TestEqual(TEXT("Party without a pawn does not grant abilities"), PartyASC->GetActivatableAbilities().Num(), 0);
			TestFalse(TEXT("Loaded character waits for a pawn"), static_cast<bool>(TActorIterator<AAetherPartyMemberStateActor>(World)));
			Controller->Possess(Pawn);
			TestEqual(TEXT("Possession attaches after loading"), Party->GetFieldCharacterPawn(), Pawn);
		}

		const int32 AbilityCount = PartyASC->GetActivatableAbilities().Num();
		TestEqual(TEXT("Four switch abilities granted"), AbilityCount, UAetherPartyComponent::MaxPartyMembers);
		TestEqual(TEXT("Party ASC avatar is the shared pawn"), PartyASC->GetAvatarActor(), static_cast<AActor*>(Pawn));
		TActorIterator<AAetherPartyMemberStateActor> It(World);
		AAetherPartyMemberStateActor* Member = It ? *It : nullptr;
		TestNotNull(TEXT("Loaded character runtime created"), Member);
		if (Member)
		{
			TestEqual(TEXT("Member ASC shares the same avatar"),
				Member->GetAetherAbilitySystemComponent()->GetAvatarActor(), static_cast<AActor*>(Pawn));
		}

		const FActiveGameplayEffectHandle Cooldown = PartyASC->ApplyGameplayEffectToSelf(
			GetDefault<UAetherGameplayEffect_SwitchCooldown>(), 1.f, PartyASC->MakeEffectContext());
		Party->AttachFieldPawn(Pawn);
		Party->InitializeParty({});
		TestEqual(TEXT("Repeated initialization preserves composition"), Party->GetPartyCharacterIds()[0], Id);
		TestEqual(TEXT("Repeated attachment does not grant again"), PartyASC->GetActivatableAbilities().Num(), AbilityCount);
		Controller->UnPossess();
		TestNull(TEXT("UnPossess detaches field pawn"), Party->GetFieldCharacterPawn());
		TestNull(TEXT("Detached party avatar cleared"), PartyASC->GetAvatarActor());
		if (Member)
		{
			TestNull(TEXT("Detached member avatar cleared"), Member->GetAetherAbilitySystemComponent()->GetAvatarActor());
		}
		TestNotNull(TEXT("Cooldown survives detachment"), PartyASC->GetActiveGameplayEffect(Cooldown));
		AAetherFieldCharacterPawn* ReplacementPawn = World->SpawnActor<AAetherFieldCharacterPawn>(
			AAetherFieldCharacterPawn::StaticClass(), FTransform::Identity, SpawnParams);
		Controller->Possess(ReplacementPawn);
		Party->DetachFieldPawn(Pawn);
		TestEqual(TEXT("Old pawn cannot detach new pawn"), Party->GetFieldCharacterPawn(), ReplacementPawn);
		TestEqual(TEXT("Reattachment does not grant again"), PartyASC->GetActivatableAbilities().Num(), AbilityCount);
		TestNotNull(TEXT("Cooldown survives reattachment"), PartyASC->GetActiveGameplayEffect(Cooldown));
		if (Member)
		{
			TestEqual(TEXT("Existing character ASC reconnects"), Member->GetAetherAbilitySystemComponent()->GetAvatarActor(),
				static_cast<AActor*>(ReplacementPawn));
		}
		World->DestroyWorld(false);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherPartyReplacementRoutingTest,
	"Aether.Party.ReplacementRequestRouting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAetherPartyReplacementRoutingTest::RunTest(const FString& Parameters)
{
	const UWorld::InitializationValues WorldValues = UWorld::InitializationValues()
		.AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false)
		.CreateAISystem(false).ShouldSimulatePhysics(false).SetTransactional(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr,
		true, ERHIFeatureLevel::Num, &WorldValues);
	AAetherPlayerState* PS = World->SpawnActor<AAetherPlayerState>();
	UAetherPartyComponent* Party = PS->GetPartyComponent();
	UAetherCharacterDefinition* First = NewObject<UAetherCharacterDefinition>(World, TEXT("FirstRoutingMember"));
	UAetherCharacterDefinition* Second = NewObject<UAetherCharacterDefinition>(World, TEXT("SecondRoutingMember"));
	const FPrimaryAssetId FirstId = First->GetPrimaryAssetId();
	const FPrimaryAssetId SecondId = Second->GetPrimaryAssetId();
	Party->InitializeParty({FirstId, SecondId});

	TestTrue(TEXT("Configured query does not require loaded members"), Party->IsPartyInitialized());
	TestTrue(TEXT("Same character is an immediate success"), Party->RequestReplacePartyMember(0, FirstId, {}));
	TestFalse(TEXT("Invalid slot is rejected"), Party->RequestReplacePartyMember(INDEX_NONE, FirstId, {}));
	TestFalse(TEXT("Unloaded members cannot be swapped"), Party->RequestReplacePartyMember(0, SecondId, {}));
	TestEqual(TEXT("Rejected swap preserves composition"), Party->GetPartyCharacterIds()[0], FirstId);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAetherFieldCharacterPawn* Pawn = World->SpawnActor<AAetherFieldCharacterPawn>(
		AAetherFieldCharacterPawn::StaticClass(), FTransform::Identity, SpawnParams);
	Party->AttachFieldPawn(Pawn);
	// No content or GameInstance is needed for routing; these members have no display mesh.
	Party->HandleInitialCharacterLoaded(First->GetPrimaryAssetId(), First);
	Party->HandleInitialCharacterLoaded(Second->GetPrimaryAssetId(), Second);
	const int32 AbilityCount = PS->GetPartyASC()->GetActivatableAbilities().Num();
	TestTrue(TEXT("Ready inactive members swap synchronously"), Party->RequestReplacePartyMember(0, SecondId, {}));
	TestEqual(TEXT("Selected character moves to requested slot"), Party->GetPartyCharacterIds()[0], SecondId);
	TestEqual(TEXT("Previous character moves to the other slot"), Party->GetPartyCharacterIds()[1], FirstId);
	TestFalse(TEXT("Swap leaves no asynchronous replacement pending"), Party->IsPartyMemberReplacementPending());
	TestEqual(TEXT("Swap does not grant party abilities again"), PS->GetPartyASC()->GetActivatableAbilities().Num(), AbilityCount);
	TestTrue(TEXT("Reverse swap remains valid"), Party->RequestReplacePartyMember(0, FirstId, {}));

	const FPrimaryAssetId OutsideId(TEXT("CharacterDefinition"), TEXT("OutsideRoutingMember"));
	TestFalse(TEXT("New member without a loading subsystem is rejected"), Party->RequestReplacePartyMember(0, OutsideId, {}));
	TestFalse(TEXT("Rejected loading request leaves no pending replacement"), Party->IsPartyMemberReplacementPending());
	TestEqual(TEXT("Rejected loading request preserves composition"), Party->GetPartyCharacterIds()[0], FirstId);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherPartySwitchPreparationTest,
	"Aether.Party.SwitchPreparation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAetherPartySwitchPreparationTest::RunTest(const FString& Parameters)
{
	const UWorld::InitializationValues WorldValues = UWorld::InitializationValues()
		.AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false)
		.CreateAISystem(false).ShouldSimulatePhysics(false).SetTransactional(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr,
		true, ERHIFeatureLevel::Num, &WorldValues);
	AAetherPlayerState* PS = World->SpawnActor<AAetherPlayerState>();
	AAetherPlayerController* Controller = World->SpawnActor<AAetherPlayerController>();
	Controller->PlayerState = PS;
	PS->SetOwner(Controller);
	UAetherPartyComponent* Party = PS->GetPartyComponent();
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAetherFieldCharacterPawn* Pawn = World->SpawnActor<AAetherFieldCharacterPawn>(
		AAetherFieldCharacterPawn::StaticClass(), FTransform::Identity, SpawnParams);
	Controller->Possess(Pawn);

	UAetherCharacterDefinition* First = NewObject<UAetherCharacterDefinition>(World, TEXT("SwitchSource"));
	UAetherCharacterDefinition* Second = NewObject<UAetherCharacterDefinition>(World, TEXT("SwitchTargetWithoutMesh"));
	Party->InitializeParty({First->GetPrimaryAssetId(), Second->GetPrimaryAssetId()});
	Party->HandleInitialCharacterLoaded(First->GetPrimaryAssetId(), First);
	Party->HandleInitialCharacterLoaded(Second->GetPrimaryAssetId(), Second);
	// Seed the active slot because this fixture intentionally has no content to display.
	Party->ActiveSlotIndex = 0;
	UAetherAbilitySystemComponent* ActiveASC = Party->GetActiveAbilitySystemComponent();
	if (!TestNotNull(TEXT("Source member ASC exists"), ActiveASC))
	{
		World->DestroyWorld(false);
		return false;
	}
	Pawn->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	const FGameplayAbilitySpecHandle JumpHandle = ActiveASC->GiveAbility(
		FGameplayAbilitySpec(UAetherGameplayAbility_Jump::StaticClass(), 1));
	TestTrue(TEXT("Source Cancel-policy ability activates"), ActiveASC->TryActivateAbility(JumpHandle));
	TestTrue(TEXT("Source ability is active before switching"),
		ActiveASC->HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy::Cancel));
	TestFalse(TEXT("Target without a mesh rejects switching"), Party->RequestSwitchPartySlot(1));
	TestEqual(TEXT("Failed preparation preserves the active slot"), Party->GetActiveSlotIndex(), 0);
	TestTrue(TEXT("Failed preparation preserves the source ability"),
		ActiveASC->HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy::Cancel));
	TestFalse(TEXT("Invalid target rejects switching"), Party->RequestSwitchPartySlot(7));
	TestTrue(TEXT("Invalid target preserves the source ability"),
		ActiveASC->HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy::Cancel));
	Controller->UnPossess();
	TestFalse(TEXT("Switching without a pawn is rejected"), Party->RequestSwitchPartySlot(1));
	TestEqual(TEXT("Missing pawn preserves the active slot"), Party->GetActiveSlotIndex(), 0);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherPartyParameterContractsTest,
	"Aether.Party.ParameterContracts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAetherPartyParameterContractsTest::RunTest(const FString& Parameters)
{
	const UWorld::InitializationValues WorldValues = UWorld::InitializationValues()
		.AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false)
		.CreateAISystem(false).ShouldSimulatePhysics(false).SetTransactional(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr,
		true, ERHIFeatureLevel::Num, &WorldValues);
	AAetherPlayerState* PS = World->SpawnActor<AAetherPlayerState>();
	UAetherPartyComponent* Party = PS->GetPartyComponent();
	UAetherCharacterDefinition* Definition = NewObject<UAetherCharacterDefinition>(World, TEXT("ContractMember"));
	const FPrimaryAssetId Id = Definition->GetPrimaryAssetId();
	Party->InitializeParty({Id, Id});
	TestFalse(TEXT("Duplicate IDs reject the whole initial composition"), Party->IsPartyInitialized());
	TestTrue(TEXT("Rejected composition does not silently compress slots"), Party->GetPartyCharacterIds().IsEmpty());
	Party->InitializeParty({Id, FPrimaryAssetId()});
	TestFalse(TEXT("Invalid IDs leave initialization available for a valid retry"), Party->IsPartyInitialized());
	Party->InitializeParty({FPrimaryAssetId(TEXT("OtherType"), TEXT("WrongType"))});
	TestFalse(TEXT("Non-character primary asset types are rejected"), Party->IsPartyInitialized());
	Party->InitializeParty({Id});
	TestTrue(TEXT("A valid composition can be configured after rejection"), Party->IsPartyInitialized());

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAetherFieldCharacterPawn* Pawn = World->SpawnActor<AAetherFieldCharacterPawn>(
		AAetherFieldCharacterPawn::StaticClass(), FTransform::Identity, SpawnParams);
	Party->AttachFieldPawn(Pawn);
	UAetherCharacterDefinition* Other = NewObject<UAetherCharacterDefinition>(World, TEXT("UnexpectedContractMember"));
	Party->HandleInitialCharacterLoaded(Other->GetPrimaryAssetId(), nullptr);
	Party->HandleInitialCharacterLoaded(Id, nullptr);
	Party->HandleInitialCharacterLoaded(Id, Other);
	TestFalse(TEXT("Failed and mismatched results do not create members"),
		static_cast<bool>(TActorIterator<AAetherPartyMemberStateActor>(World)));
	Party->HandleInitialCharacterLoaded(Id, Definition);
	TActorIterator<AAetherPartyMemberStateActor> It(World);
	AAetherPartyMemberStateActor* Member = It ? *It : nullptr;
	if (!TestNotNull(TEXT("The matching result creates a member without a GameInstance lookup"), Member))
	{
		World->DestroyWorld(false);
		return false;
	}
	Party->HandleInitialCharacterLoaded(Id, Definition);
	int32 MemberCount = 0;
	for (TActorIterator<AAetherPartyMemberStateActor> MemberIt(World); MemberIt; ++MemberIt)
	{
		++MemberCount;
	}
	TestEqual(TEXT("Duplicate completion does not create another member"), MemberCount, 1);

	AAetherPartyMemberStateActor* StandaloneMember = World->SpawnActor<AAetherPartyMemberStateActor>();
	TestFalse(TEXT("Initial setup rejects a missing definition"), StandaloneMember->InitializeCharacterRuntime(nullptr, Pawn));
	TestFalse(TEXT("Initial setup rejects a missing avatar"), StandaloneMember->InitializeCharacterRuntime(Definition, nullptr));
	UAetherCharacterDefinition* RuntimeDefinition = NewObject<UAetherCharacterDefinition>(World, TEXT("DirectRuntimeDefinition"));
	USkeletalMesh* Mesh = NewObject<USkeletalMesh>(RuntimeDefinition);
	RuntimeDefinition->DisplayMesh = Mesh;
	UAetherAbilitySet* AbilitySet = NewObject<UAetherAbilitySet>(RuntimeDefinition);
	FAetherAbilitySet_GameplayAbility JumpAbility;
	JumpAbility.Ability = UAetherGameplayAbility_Jump::StaticClass();
	AbilitySet->Abilities.Add(JumpAbility);
	RuntimeDefinition->AbilitySets.Add(AbilitySet);
	TestTrue(TEXT("Loaded definition initializes directly without an asset subsystem"),
		StandaloneMember->InitializeCharacterRuntime(RuntimeDefinition, Pawn));
	TestEqual(TEXT("Appearance comes from the supplied definition"), StandaloneMember->GetSkeletalMesh(), Mesh);
	UAetherAbilitySystemComponent* ASC = StandaloneMember->GetAetherAbilitySystemComponent();
	TestEqual(TEXT("The supplied ability set is granted"), ASC->GetActivatableAbilities().Num(), 1);
	TestFalse(TEXT("Repeated setup is rejected"), StandaloneMember->InitializeCharacterRuntime(Other, Pawn));
	StandaloneMember->SetAvatarActor(nullptr);
	TestNull(TEXT("Avatar can be cleared without recreating the ASC"), ASC->GetAvatarActor());
	StandaloneMember->SetAvatarActor(Pawn);
	TestEqual(TEXT("Avatar reconnect preserves the member owner"), ASC->GetOwnerActor(), static_cast<AActor*>(StandaloneMember));
	TestEqual(TEXT("Avatar reconnect does not grant abilities again"), ASC->GetActivatableAbilities().Num(), 1);

	const TWeakObjectPtr<UAetherCharacterDefinition> RetainedDefinition = RuntimeDefinition;
	CollectGarbage(RF_NoFlags);
	TestTrue(TEXT("The member retains its definition across garbage collection"), RetainedDefinition.IsValid());
	World->DestroyWorld(false);
	return true;
}

#endif
