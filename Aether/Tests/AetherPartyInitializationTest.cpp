#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Aether/AetherGameMode.h"
#include "Aether/AetherGameplayTags.h"
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
#include "UObject/UnrealType.h"

namespace
{
void ConfigureDefinition(AAetherPlayerState* PS, UAetherCharacterDefinition* Definition)
{
	FObjectProperty* Property = FindFProperty<FObjectProperty>(PS->GetClass(), TEXT("InitialPlayerData"));
	UAetherInitialPlayerData* Data = Cast<UAetherInitialPlayerData>(Property->GetObjectPropertyValue_InContainer(PS));
	if (!Data)
	{
		Data = NewObject<UAetherInitialPlayerData>(PS);
		Property->SetObjectPropertyValue_InContainer(PS, Data);
	}
	Data->CharacterDefinitions.AddUnique(Definition);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherPartyInitializationTest,
	"Aether.Party.OneTimeAbilitySystemInitialization",
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
		Controller->SetAsLocalPlayerController();
		Controller->PlayerState = PS;
		PS->SetOwner(Controller);
		UAetherPartyComponent* Party = PS->GetPartyComponent();
		UAetherAbilitySystemComponent* PartyASC = PS->GetPartyASC();
		UAetherAbilitySet* PartyAbilitySet = NewObject<UAetherAbilitySet>(Party);
		FAetherAbilitySet_GameplayAbility JumpAbility;
		JumpAbility.Ability = UAetherGameplayAbility_Jump::StaticClass();
		JumpAbility.InputTag = AetherGameplayTags::InputTag_Jump;
		PartyAbilitySet->Abilities.Add(JumpAbility);
		Party->PartyAbilitySet = PartyAbilitySet;
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AAetherFieldCharacterPawn* Pawn = World->SpawnActor<AAetherFieldCharacterPawn>(
			AAetherFieldCharacterPawn::StaticClass(), FTransform::Identity, SpawnParams);
		UAetherCharacterDefinition* Definition = NewObject<UAetherCharacterDefinition>(
			World, bPawnFirst ? TEXT("PawnFirstDefinition") : TEXT("PartyFirstDefinition"));
		const FPrimaryAssetId Id = Definition->GetPrimaryAssetId();
		ConfigureDefinition(PS, Definition);

		if (bPawnFirst)
		{
			Controller->Possess(Pawn);
			TestEqual(TEXT("Possession supplies the shared field pawn"), Party->GetFieldCharacterPawn(), Pawn);
			TestEqual(TEXT("Pawn readiness initializes Party ASC"), PartyASC->GetAvatarActor(), static_cast<AActor*>(Pawn));
			TestEqual(TEXT("Party abilities are granted when the pawn is ready"), PartyASC->GetActivatableAbilities().Num(), 5);
		}
		Party->InitializeParty({Id});
		if (!bPawnFirst)
		{
			TestEqual(TEXT("Party without a pawn does not grant abilities"), PartyASC->GetActivatableAbilities().Num(), 0);
			TestNull(TEXT("Party ASC waits for the shared pawn"), PartyASC->GetAvatarActor());
			TestFalse(TEXT("Configured character waits for a pawn"), static_cast<bool>(TActorIterator<AAetherPartyMemberStateActor>(World)));
			Controller->Possess(Pawn);
			TestEqual(TEXT("Possession initializes configured members"), Party->GetFieldCharacterPawn(), Pawn);
		}

		const int32 AbilityCount = PartyASC->GetActivatableAbilities().Num();
		TestEqual(TEXT("Party AbilitySet and four switch abilities granted"), AbilityCount, UAetherPartyComponent::MaxPartyMembers + 1);
		TestEqual(TEXT("Party ASC owner is the PlayerState"), PartyASC->GetOwnerActor(), static_cast<AActor*>(PS));
		TestEqual(TEXT("Party ASC avatar is the shared pawn"), PartyASC->GetAvatarActor(), static_cast<AActor*>(Pawn));
		TActorIterator<AAetherPartyMemberStateActor> It(World);
		AAetherPartyMemberStateActor* Member = It ? *It : nullptr;
		TestNotNull(TEXT("Directly configured character runtime created"), Member);
		if (Member)
		{
			TestEqual(TEXT("Member ASC shares the same avatar"),
				Member->GetAetherAbilitySystemComponent()->GetAvatarActor(), static_cast<AActor*>(Pawn));
		}

		const FActiveGameplayEffectHandle Cooldown = PartyASC->ApplyGameplayEffectToSelf(
			GetDefault<UAetherGameplayEffect_SwitchCooldown>(), 1.f, PartyASC->MakeEffectContext());
		Pawn->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		PartyASC->AbilityInputPressed(AetherGameplayTags::InputTag_Jump);
		Party->InitializeFieldPawn(Pawn);
		Party->InitializeParty({});
		Party->TryInitializePartyRuntime();
		TestEqual(TEXT("Repeated initialization preserves composition"), Party->GetPartyCharacterIds()[0], Id);
		TestEqual(TEXT("Readiness checks do not grant again"), PartyASC->GetActivatableAbilities().Num(), AbilityCount);
		PartyASC->ProcessInputs();
		TestTrue(TEXT("Readiness checks preserve queued input instead of reinitializing ActorInfo"),
			PartyASC->HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy::Cancel));
		AAetherFieldCharacterPawn* ReplacementPawn = World->SpawnActor<AAetherFieldCharacterPawn>(
			AAetherFieldCharacterPawn::StaticClass(), FTransform::Identity, SpawnParams);
		Party->InitializeFieldPawn(ReplacementPawn);
		TestEqual(TEXT("Another pawn cannot replace the shared pawn"), Party->GetFieldCharacterPawn(), Pawn);
		TestEqual(TEXT("Party avatar remains the original pawn"), PartyASC->GetAvatarActor(), static_cast<AActor*>(Pawn));
		TestNotNull(TEXT("Cooldown survives repeated readiness checks"), PartyASC->GetActiveGameplayEffect(Cooldown));
		if (Member)
		{
			TestEqual(TEXT("Member avatar remains the original pawn"), Member->GetAetherAbilitySystemComponent()->GetAvatarActor(),
				static_cast<AActor*>(Pawn));
		}
		// This isolated world does not begin play automatically; enter the component lifecycle before teardown.
		Party->RegisterAllComponentTickFunctions(true);
		Party->BeginPlay();
		Party->EndPlay(EEndPlayReason::EndPlayInEditor);
		TestNull(TEXT("Party teardown clears its avatar"), PartyASC->GetAvatarActor());
		TestFalse(TEXT("Party teardown cancels the active ability"),
			PartyASC->HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy::Cancel));
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
	ConfigureDefinition(PS, First);
	ConfigureDefinition(PS, Second);
	Party->InitializeParty({FirstId, SecondId});

	TestTrue(TEXT("Configured query does not require initialized members"), Party->IsPartyInitialized());
	TestTrue(TEXT("Same character is an immediate success"), Party->RequestReplacePartyMember(0, FirstId, {}));
	TestFalse(TEXT("Invalid slot is rejected"), Party->RequestReplacePartyMember(INDEX_NONE, FirstId, {}));
	TestFalse(TEXT("Uninitialized members cannot be swapped"), Party->RequestReplacePartyMember(0, SecondId, {}));
	TestEqual(TEXT("Rejected swap preserves composition"), Party->GetPartyCharacterIds()[0], FirstId);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAetherFieldCharacterPawn* Pawn = World->SpawnActor<AAetherFieldCharacterPawn>(
		AAetherFieldCharacterPawn::StaticClass(), FTransform::Identity, SpawnParams);
	Party->InitializeFieldPawn(Pawn);
	// No content or GameInstance is needed for routing; these members have no display mesh.
	const int32 AbilityCount = PS->GetPartyASC()->GetActivatableAbilities().Num();
	TestTrue(TEXT("Ready inactive members swap synchronously"), Party->RequestReplacePartyMember(0, SecondId, {}));
	TestEqual(TEXT("Selected character moves to requested slot"), Party->GetPartyCharacterIds()[0], SecondId);
	TestEqual(TEXT("Previous character moves to the other slot"), Party->GetPartyCharacterIds()[1], FirstId);
	TestEqual(TEXT("Swap does not grant party abilities again"), PS->GetPartyASC()->GetActivatableAbilities().Num(), AbilityCount);
	TestTrue(TEXT("Reverse swap remains valid"), Party->RequestReplacePartyMember(0, FirstId, {}));

	const FPrimaryAssetId OutsideId(TEXT("CharacterDefinition"), TEXT("OutsideRoutingMember"));
	TestFalse(TEXT("New member without a configured Definition is rejected"), Party->RequestReplacePartyMember(0, OutsideId, {}));
	TestEqual(TEXT("Missing Definition preserves composition"), Party->GetPartyCharacterIds()[0], FirstId);
	UAetherCharacterDefinition* Third = NewObject<UAetherCharacterDefinition>(World, TEXT("ThirdRoutingMember"));
	Third->DisplayMesh = NewObject<USkeletalMesh>(Third);
	ConfigureDefinition(PS, Third);
	TestTrue(TEXT("New configured member replaces immediately"), Party->RequestReplacePartyMember(1, Third->GetPrimaryAssetId(), {}));
	TestEqual(TEXT("Direct replacement updates its slot"), Party->GetPartyCharacterIds()[1], Third->GetPrimaryAssetId());
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
	ConfigureDefinition(PS, First);
	ConfigureDefinition(PS, Second);
	Party->InitializeParty({First->GetPrimaryAssetId(), Second->GetPrimaryAssetId()});
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
	// Jump cancellation expects a live avatar; finish it before testing an invalid pawn.
	ActiveASC->CancelAllAbilities();
	Pawn->Destroy();
	TestFalse(TEXT("Switching without a pawn is rejected"), Party->RequestSwitchPartySlot(1));
	TestEqual(TEXT("Missing pawn preserves the active slot"), Party->GetActiveSlotIndex(), 0);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherPartyParameterContractsTest,
	"Aether.Party.DirectDefinitionContracts",
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
	ConfigureDefinition(PS, Definition);
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
	Party->InitializeFieldPawn(Pawn);
	UAetherCharacterDefinition* Other = NewObject<UAetherCharacterDefinition>(World, TEXT("UnexpectedContractMember"));
	TestNull(TEXT("Unconfigured IDs have no Definition"), PS->GetCharacterDefinition(Other->GetPrimaryAssetId()));
	TestEqual(TEXT("Definition lookup uses direct configuration"), PS->GetCharacterDefinition(Id), Definition);
	TActorIterator<AAetherPartyMemberStateActor> It(World);
	AAetherPartyMemberStateActor* Member = It ? *It : nullptr;
	if (!TestNotNull(TEXT("Direct configuration creates a member without a GameInstance lookup"), Member))
	{
		World->DestroyWorld(false);
		return false;
	}
	Party->InitializeParty({Id});
	int32 MemberCount = 0;
	for (TActorIterator<AAetherPartyMemberStateActor> MemberIt(World); MemberIt; ++MemberIt)
	{
		++MemberCount;
	}
	TestEqual(TEXT("Repeated setup does not create another member"), MemberCount, 1);

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
	TestTrue(TEXT("Direct definition initializes without an asset subsystem"),
		StandaloneMember->InitializeCharacterRuntime(RuntimeDefinition, Pawn));
	TestEqual(TEXT("Appearance comes from the supplied definition"), StandaloneMember->GetSkeletalMesh(), Mesh);
	UAetherAbilitySystemComponent* ASC = StandaloneMember->GetAetherAbilitySystemComponent();
	TestEqual(TEXT("The supplied ability set is granted"), ASC->GetActivatableAbilities().Num(), 1);
	TestFalse(TEXT("Repeated setup is rejected"), StandaloneMember->InitializeCharacterRuntime(Other, Pawn));
	TestEqual(TEXT("Rejected setup preserves the member owner"), ASC->GetOwnerActor(), static_cast<AActor*>(StandaloneMember));
	TestEqual(TEXT("Rejected setup preserves the original avatar"), ASC->GetAvatarActor(), static_cast<AActor*>(Pawn));
	TestEqual(TEXT("Rejected setup does not grant abilities again"), ASC->GetActivatableAbilities().Num(), 1);

	const TWeakObjectPtr<UAetherCharacterDefinition> RetainedDefinition = RuntimeDefinition;
	CollectGarbage(RF_NoFlags);
	TestTrue(TEXT("The member retains its definition across garbage collection"), RetainedDefinition.IsValid());
	World->DestroyWorld(false);
	return true;
}

#endif
