#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "AIController.h"
#include "Aether/AetherGameplayTags.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AetherCharacterDefinition.h"
#include "Aether/AbilitySystem/Abilities/AetherGameplayAbility_HealingField.h"
#include "Aether/AbilitySystem/Actors/AetherHealingFieldActor.h"
#include "Aether/AbilitySystem/AttributeSet/AetherBaseAttributeSet.h"
#include "Aether/PartySystem/AetherPartyMemberStateActor.h"
#include "Aether/PartySystem/AetherPartyComponent.h"
#include "Aether/PartySystem/AetherFieldCharacterPawn.h"
#include "Aether/Player/AetherPlayerState.h"
#include "Aether/Player/AetherPlayerController.h"
#include "Aether/Input/AetherInputConfig.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Blueprint.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"

namespace HealingFieldTest
{
struct FFixture
{
	UWorld* World;
	AAetherPlayerState* PS;
	AAetherFieldCharacterPawn* Pawn;
	AAetherPlayerController* Controller;
	UAetherCharacterDefinition* Definition;
	UAetherPartyComponent* Party;
	UAetherAbilitySystemComponent* ASC;
	FGameplayAbilitySpecHandle Handle;

	FFixture(TSubclassOf<UAetherGameplayAbility> AbilityClass = UAetherGameplayAbility_HealingField::StaticClass(), int32 MemberCount = 1)
	{
		const auto Values = UWorld::InitializationValues().AllowAudioPlayback(false)
			.CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false)
			.ShouldSimulatePhysics(false).SetTransactional(false);
		World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr,
			true, ERHIFeatureLevel::Num, &Values);
		// Enter the actual component initialization lifecycle (registers native AttributeSets).
		World->InitializeActorsForPlay(FURL());
		AActor* Floor = World->SpawnActor<AActor>();
		UBoxComponent* Box = NewObject<UBoxComponent>(Floor);
		Floor->SetRootComponent(Box);
		Box->SetBoxExtent(FVector(5000, 5000, 10));
		Box->SetCollisionProfileName(TEXT("BlockAll"));
		Box->RegisterComponent();
		Floor->SetActorLocation(FVector(0, 0, -10));

		PS = World->SpawnActor<AAetherPlayerState>();
		Controller = World->SpawnActor<AAetherPlayerController>();
		Controller->SetAsLocalPlayerController();
		Controller->PlayerState = PS;
		PS->SetOwner(Controller);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Pawn = World->SpawnActor<AAetherFieldCharacterPawn>(FVector(0, 0, 88), FRotator::ZeroRotator, Params);
		Controller->Possess(Pawn);
		Party = PS->GetPartyComponent();
		Definition = NewObject<UAetherCharacterDefinition>(World);
		Definition->DisplayMesh = NewObject<USkeletalMesh>(World);
		UAetherAbilitySet* Set = NewObject<UAetherAbilitySet>(World);
		FAetherAbilitySet_GameplayAbility Entry;
		Entry.Ability = AbilityClass;
		Entry.InputTag = AetherGameplayTags::InputTag_HealingField;
		Set->Abilities.Add(Entry);
		Definition->AbilitySets.Add(Set);
		UAetherInitialPlayerData* Data = NewObject<UAetherInitialPlayerData>(PS);
		Data->CharacterDefinitions.Add(Definition);
        TArray<FPrimaryAssetId> InitialParty{Definition->GetPrimaryAssetId()};
        for (int32 Index = 1; Index < MemberCount; ++Index)
        {
            auto* Other = NewObject<UAetherCharacterDefinition>(World);
            Other->DisplayMesh = Definition->DisplayMesh;
            Data->CharacterDefinitions.Add(Other);
            InitialParty.Add(Other->GetPrimaryAssetId());
        }
        Data->InitialParty = InitialParty;
		FindFProperty<FObjectProperty>(PS->GetClass(), TEXT("InitialPlayerData"))->SetObjectPropertyValue_InContainer(PS, Data);
		Party->InitializeParty(InitialParty);
		ASC = Party->GetActiveAbilitySystemComponent();
		Handle = ASC->GetActivatableAbilities()[0].Handle;
		World->BeginPlay();
		Pawn->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		// Establish a timer frame before input; newly scheduled timers then start now.
		++GFrameCounter;
		World->GetTimerManager().Tick(0.f);
	}
	~FFixture()
	{
		World->DestroyWorld(false);
	}
	UAetherGameplayAbility_HealingField* Ability(UAetherAbilitySystemComponent* Target = nullptr)
	{
		return Cast<UAetherGameplayAbility_HealingField>((Target ? Target : ASC)
			->GetActivatableAbilities()[0].GetPrimaryInstance());
	}
	void Press()
	{
		ASC->AbilityInputPressed(AetherGameplayTags::InputTag_HealingField);
		ASC->ProcessInputs();
		ASC->AbilityInputReleased(AetherGameplayTags::InputTag_HealingField);
		ASC->ProcessInputs();
	}
	void Advance(float Seconds)
	{
		// Real timer manager, independently of rendering/character simulation.
		for (int32 Index = 0; Index < FMath::CeilToInt(Seconds / 0.05f); ++Index)
		{
			++GFrameCounter;
			World->GetTimerManager().Tick(0.05f);
		}
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldLifecycleTest,
	"Aether.HealingField.Stage1.Lifecycle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldLifecycleTest::RunTest(const FString& Parameters)
{
	using namespace HealingFieldTest;
	FFixture F;
	TestEqual(TEXT("Granted once"), F.ASC->GetActivatableAbilities().Num(), 1);
	F.Press();
	auto* Ability = F.Ability();
	AAetherHealingFieldActor* First = Ability->GetSpawnedField();
	if (!TestNotNull(TEXT("Input creates field"), First)) return false;
	TestFalse(TEXT("Ability ends immediately"), F.ASC->FindAbilitySpecFromHandle(F.Handle)->IsActive());
	TestTrue(TEXT("Field is visible"), !First->IsHidden());
	TestEqual(TEXT("Original ASC retained"), First->GetCasterASC(), static_cast<UAbilitySystemComponent*>(F.ASC));
	TestEqual(TEXT("Explicit source party retained"), First->GetSourceParty(), F.Party);
	TestTrue(TEXT("Ground location"), First->GetActorLocation().Equals(FVector(0,0,2), 0.5f));
	F.Advance(5.f);
	TestTrue(TEXT("Survives after ability ends"), IsValid(First));
	F.Pawn->SetActorLocation(FVector(200,0,88));
	F.Press();
	AAetherHealingFieldActor* Second = Ability->GetSpawnedField();
	TestTrue(TEXT("Reuses ability instance"), Ability == F.Ability());
	TestTrue(TEXT("Replaces old field"), Second && Second != First && !IsValid(First));
	if (!Second) return false;
	TestTrue(TEXT("Uses new position"), Second->GetActorLocation().Equals(FVector(200,0,2), 0.5f));
	F.Advance(5.2f);
	TestTrue(TEXT("Old expiration cannot remove replacement"), IsValid(Second));
	F.Advance(4.5f);
	TestTrue(TEXT("Still present before own deadline"), IsValid(Second));
	F.Advance(0.5f);
	TestFalse(TEXT("Expires after ten seconds"), IsValid(Second));
	TestNull(TEXT("Weak reference invalidates"), Ability->GetSpawnedField());
	F.Press();
	TestNotNull(TEXT("Recasts after natural expiration"), Ability->GetSpawnedField());
	TestEqual(TEXT("Full health remains capped"), F.ASC->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute()), 100.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldCastConditionsTest,
	"Aether.HealingField.Stage1.CastConditions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldCastConditionsTest::RunTest(const FString& Parameters)
{
	HealingFieldTest::FFixture F;
	F.Press();
	auto* Original = F.Ability()->GetSpawnedField();
	if (!TestNotNull(TEXT("Initial field"), Original)) return false;
	F.Pawn->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	F.Press();
	TestEqual(TEXT("Airborne request preserves field"), F.Ability()->GetSpawnedField(), Original);
	F.Pawn->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	F.Pawn->SetActorLocation(FVector(10000,0,88));
	F.Press();
	TestEqual(TEXT("No ground preserves field"), F.Ability()->GetSpawnedField(), Original);
	F.Pawn->SetActorLocation(FVector(0,0,88));
	F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 0);
	F.Press();
	TestNull(TEXT("Dead caster cannot recreate field"), F.Ability()->GetSpawnedField());
	TestFalse(TEXT("Caster death removed previous field"), IsValid(Original));
	TestFalse(TEXT("Rejected attempt is not left active"), F.ASC->FindAbilitySpecFromHandle(F.Handle)->IsActive());
	F.ASC->ClearAbility(F.Handle);
	if (IsValid(Original)) Original->Destroy();
	F.Press();
	TestFalse(TEXT("Input without granted ability creates no field"), static_cast<bool>(TActorIterator<AAetherHealingFieldActor>(F.World)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldNPCSmokeTest,
	"Aether.HealingField.Stage1.NPCIsolation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldNPCSmokeTest::RunTest(const FString& Parameters)
{
	HealingFieldTest::FFixture F;
	F.Press();
	auto* PlayerField = F.Ability()->GetSpawnedField();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	auto* NPC = F.World->SpawnActor<AAetherFieldCharacterPawn>(FVector(1000,0,88), FRotator::ZeroRotator, Params);
	auto* AI = F.World->SpawnActor<AAIController>();
	AI->Possess(NPC);
	NPC->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	auto* Member = F.World->SpawnActor<AAetherPartyMemberStateActor>();
	TestTrue(TEXT("Same definition, separate NPC runtime"), Member->InitializeCharacterRuntime(F.Definition, NPC));
	auto* ASC = Member->GetAetherAbilitySystemComponent();
	TestNull(TEXT("No fake player controller"), ASC->AbilityActorInfo->PlayerController.Get());
	TestTrue(TEXT("AIController possesses NPC"), NPC->GetController() == AI);
	const auto Handle = ASC->FindAbilitySpecFromClass(UAetherGameplayAbility_HealingField::StaticClass())->Handle;
	TestTrue(TEXT("NPC activates same LocalOnly ability"), ASC->TryActivateAbility(Handle));
	auto* NPCAbility = F.Ability(ASC);
	auto* NPCField = NPCAbility->GetSpawnedField();
	if (!TestNotNull(TEXT("NPC field created"), NPCField)) return false;
	TestTrue(TEXT("Ability instances separate"), NPCAbility != F.Ability());
	TestTrue(TEXT("Fields coexist"), IsValid(PlayerField) && NPCField != PlayerField);
	TestEqual(TEXT("NPC ASC retained"), NPCField->GetCasterASC(), static_cast<UAbilitySystemComponent*>(ASC));
	TestNull(TEXT("Standalone NPC does not use player party"), NPCField->GetSourceParty());
	ASC->TryActivateAbility(Handle);
	TestFalse(TEXT("NPC replaces own field"), IsValid(NPCField));
	TestTrue(TEXT("Player field untouched"), IsValid(PlayerField));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldContentTest,
	"Aether.HealingField.Stage1.Content", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldContentTest::RunTest(const FString& Parameters)
{
	auto* AbilityBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Aether/Ability/HealingField/GA_HealingField.GA_HealingField"));
	auto* FieldBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Aether/Ability/HealingField/BP_HealingField.BP_HealingField"));
	if (!TestNotNull(TEXT("Saved ability Blueprint"), AbilityBP) || !TestNotNull(TEXT("Saved field Blueprint"), FieldBP)) return false;
	TestTrue(TEXT("Ability Blueprint compiled"), AbilityBP->Status == BS_UpToDate);
	TestTrue(TEXT("Field Blueprint compiled"), FieldBP->Status == BS_UpToDate);
	const auto AbilityClass = TSubclassOf<UAetherGameplayAbility>(AbilityBP->GeneratedClass);
	for (const TCHAR* Name : {TEXT("DA_Yuito"), TEXT("DA_Hanabi"), TEXT("DA_Tsugumi"), TEXT("DA_Luka"), TEXT("DA_Gemma")})
	{
		const FString Path = FString::Printf(TEXT("/Game/Aether/Characters/%s.%s"), Name, Name);
		auto* Definition = LoadObject<UAetherCharacterDefinition>(nullptr, *Path);
		if (!TestNotNull(Path, Definition)) return false;
		int32 Count = 0;
		for (const UAetherAbilitySet* Set : Definition->AbilitySets)
			for (const auto& Grant : Set->Abilities)
				if (Grant.Ability == AbilityClass)
				{
					++Count;
					TestEqual(TEXT("Grant input tag"), Grant.InputTag, FGameplayTag(AetherGameplayTags::InputTag_HealingField));
				}
		TestEqual(Path + TEXT(" unique grant"), Count, FString(Name) == TEXT("DA_Yuito") ? 1 : 0);
	}
	auto* Action = LoadObject<UInputAction>(nullptr, TEXT("/Game/Aether/Input/IA_HealingField.IA_HealingField"));
	auto* IMC = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Aether/Input/IMC_Default.IMC_Default"));
	auto* Config = LoadObject<UAetherInputConfig>(nullptr, TEXT("/Game/Aether/Input/InputConfig_Default.InputConfig_Default"));
	if (!TestNotNull(TEXT("Action"), Action) || !TestNotNull(TEXT("IMC"), IMC) || !TestNotNull(TEXT("Input config"), Config)) return false;
	int32 Mappings = 0, Bindings = 0;
	for (const auto& Mapping : IMC->GetMappings())
		if (Mapping.Action == Action)
		{
			++Mappings;
			TestEqual(TEXT("Mapped physical E key"), Mapping.Key, EKeys::E);
		}
	for (const auto& Binding : Config->AbilityInputActions)
		if (Binding.InputAction == Action)
		{
			++Bindings;
			TestEqual(TEXT("Bound input tag"), Binding.InputTag, FGameplayTag(AetherGameplayTags::InputTag_HealingField));
		}
	TestEqual(TEXT("One E mapping"), Mappings, 1);
	TestEqual(TEXT("One input binding"), Bindings, 1);
	HealingFieldTest::FFixture F(AbilityClass);
	F.Press();
	auto* Field = F.Ability()->GetSpawnedField();
	if (!TestNotNull(TEXT("Actual Blueprint ability creates field"), Field)) return false;
	TestEqual(TEXT("Uses saved field Blueprint"), Field->GetClass(), FieldBP->GeneratedClass.Get());
	const auto* Visual = Field->FindComponentByClass<UStaticMeshComponent>();
	TestNotNull(TEXT("Visible mesh"), Visual);
	if (Visual)
	{
		TestEqual(TEXT("500cm radius scale"), Visual->GetRelativeScale3D().X, 10.0);
		TestEqual(TEXT("Green material"), Visual->GetMaterial(0)->GetName(), FString(TEXT("M_HealingField")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldScheduleTest,
    "Aether.HealingField.Stage2.ScheduleAndSnapshot", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldScheduleTest::RunTest(const FString& Parameters)
{
    HealingFieldTest::FFixture F;
    F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetMaxHealthAttribute(), 1000.f);
    F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 1.f);
    TArray<float> Values;
    TArray<float> Times;
    float Clock = 0.f;
    const FDelegateHandle Listener = F.ASC->GetGameplayAttributeValueChangeDelegate(
        UAetherBaseAttributeSet::GetHealthAttribute()).AddLambda(
        [&](const FOnAttributeChangeData& Data) { Values.Add(Data.NewValue); Times.Add(Clock); });
    F.Press();
    TestEqual(TEXT("Immediate tick"), Values.Num(), 1);
    TestEqual(TEXT("Five percent snapshot"), Values.Last(), 51.f);
    F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetMaxHealthAttribute(), 2000.f);
    for (int32 Tick = 1; Tick <= 9; ++Tick)
    {
        for (int32 Frame = 0; Frame < 100; ++Frame)
        {
            Clock += 0.01f;
            ++GFrameCounter;
            F.World->GetTimerManager().Tick(0.01f);
        }
        // Engine dispatches on the frame after a deadline, not at subframe precision.
        Clock += 0.001f;
        ++GFrameCounter;
        F.World->GetTimerManager().Tick(0.001f);
        TestEqual(FString::Printf(TEXT("Application count at second %d"), Tick), Values.Num(), Tick + 1);
        TestEqual(TEXT("Snapshot unchanged"), Values.Last(), 1.f + 50.f * (Tick + 1));
        if (Times.Num() > Tick)
            TestTrue(TEXT("Applied on first frame near its scheduled second"), FMath::Abs(Times[Tick] - Tick) < 0.02f);
    }
    F.Advance(1.f);
    TestEqual(TEXT("No tenth-second application"), Values.Num(), 10);
    F.Advance(2.f);
    TestEqual(TEXT("No late application"), Values.Num(), 10);
    F.ASC->GetGameplayAttributeValueChangeDelegate(UAetherBaseAttributeSet::GetHealthAttribute()).Remove(Listener);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldRangeTest,
    "Aether.HealingField.Stage2.RangeAndHealth", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldRangeTest::RunTest(const FString& Parameters)
{
    HealingFieldTest::FFixture F;
    auto SetHP = [&](float HP) { F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), HP); };
    auto HP = [&]() { return F.ASC->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute()); };
    SetHP(10.f);
    F.Press();
    F.Advance(0.05f);
    TestEqual(TEXT("Self heals immediately"), HP(), 15.f);
    F.Pawn->SetActorLocation(FVector(500.1,0,88));
    F.Advance(1.f);
    TestEqual(TEXT("Outside radius skipped"), HP(), 15.f);
    F.Pawn->SetActorLocation(FVector(500,0,88));
    F.Advance(0.4f);
    TestEqual(TEXT("Entry does not heal immediately"), HP(), 15.f);
    F.Advance(0.6f);
    TestEqual(TEXT("Radius boundary included at next tick"), HP(), 20.f);
    F.Pawn->SetActorLocation(FVector(0,0,188.1));
    F.Advance(1.f);
    TestEqual(TEXT("Above height tolerance skipped"), HP(), 20.f);
    F.Pawn->SetActorLocation(FVector(0,0,188));
    F.Advance(1.f);
    TestEqual(TEXT("Height boundary included"), HP(), 25.f);
    F.Pawn->SetActorLocation(FVector(0,0,-12.1));
    F.Advance(1.f);
    TestEqual(TEXT("Below height tolerance skipped"), HP(), 25.f);
    F.Pawn->SetActorLocation(FVector(0,0,-12));
    F.Advance(1.f);
    TestEqual(TEXT("Lower height boundary included"), HP(), 30.f);
    F.Pawn->SetActorLocation(FVector(0,0,88));
    SetHP(98.5f);
    F.Advance(1.f);
    TestEqual(TEXT("Integer increase leaves fractional deficit"), HP(), 99.5f);
    F.Advance(1.f);
    TestEqual(TEXT("Deficit below one does not heal"), HP(), 99.5f);
    SetHP(0.f);
    F.Advance(1.f);
    TestEqual(TEXT("Dead target is not revived"), HP(), 0.f);
    TestNull(TEXT("Caster death removes field immediately"), F.Ability()->GetSpawnedField());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldDelayTest,
    "Aether.HealingField.Stage2.DelayAndRemoval", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldDelayTest::RunTest(const FString& Parameters)
{
    HealingFieldTest::FFixture F;
    auto HP = [&]() { return F.ASC->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute()); };
    F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 1.f);
    F.Press();
    ++GFrameCounter;
    F.World->GetTimerManager().Tick(3.4f);
    TestEqual(TEXT("Hitch applies only one current opportunity"), HP(), 11.f);
    F.Advance(0.7f);
    TestEqual(TEXT("Next tick keeps original phase"), HP(), 16.f);
    ++GFrameCounter;
    F.World->GetTimerManager().Tick(10.f);
    TestEqual(TEXT("Hitch past expiry never heals"), HP(), 16.f);
    TestNull(TEXT("Expired field"), F.Ability()->GetSpawnedField());
    F.Press();
    F.Advance(0.5f);
    F.Press();
    TestEqual(TEXT("Two recasts heal immediately"), HP(), 26.f);
    F.Advance(0.6f);
    TestEqual(TEXT("Removed field cannot tick"), HP(), 26.f);
    F.Advance(0.5f);
    TestEqual(TEXT("Replacement ticks once"), HP(), 31.f);
    F.Ability()->GetSpawnedField()->Destroy();
    F.Advance(2.f);
    TestEqual(TEXT("Explicit destruction stops healing"), HP(), 31.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldRoundingTest,
    "Aether.HealingField.Stage2.RoundingAndContent", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldRoundingTest::RunTest(const FString& Parameters)
{
    auto* BP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Aether/Ability/HealingField/GA_HealingField.GA_HealingField"));
    if (!TestNotNull(TEXT("Actual ability asset"), BP)) return false;
    HealingFieldTest::FFixture F(TSubclassOf<UAetherGameplayAbility>(BP->GeneratedClass));
    auto HP = [&]() { return F.ASC->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute()); };
    F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetMaxHealthAttribute(), 999.f);
    F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 1.f);
    F.Press();
    TestEqual(TEXT("Saved Blueprint inherits five percent, rounded down"), HP(), 50.f);
    F.Advance(0.05f);
    F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 980.f);
    F.Advance(1.f);
    TestEqual(TEXT("Clamped to integer maximum"), HP(), 999.f);
    F.Advance(1.f);
    TestEqual(TEXT("Full health stays capped"), HP(), 999.f);
    F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetMaxHealthAttribute(), 10.f);
    F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 1.f);
    F.Press();
    F.Advance(1.f);
    TestEqual(TEXT("Rounded zero produces no heal"), HP(), 1.f);
    return true;
}

namespace HealingFieldTest
{
// Production party initialization and public switching API only; no private member injection.
struct FSwitchFixture : FFixture
{
    TArray<UAetherAbilitySystemComponent*> Members;
    struct FHealingEvent { int32 Slot; double Time; float Amount; };
    TArray<FHealingEvent> Events;
    TArray<FDelegateHandle> Listeners;
    double Clock = 0.0;
    bool bReady = true;

    FSwitchFixture(int32 MemberCount = 3) : FFixture(UAetherGameplayAbility_HealingField::StaticClass(), MemberCount)
    {
        for (int32 Slot = 0; Slot < MemberCount; ++Slot)
        {
            bReady &= Party->RequestSwitchPartySlot(Slot);
            Members.Add(Party->GetActiveAbilitySystemComponent());
        }
        bReady &= Party->RequestSwitchPartySlot(0);
        TSet<UAetherAbilitySystemComponent*> UniqueMembers;
        for (auto* Member : Members)
        {
            bReady &= IsValid(Member) && !UniqueMembers.Contains(Member);
            UniqueMembers.Add(Member);
        }
        if (!bReady) return;
        for (int32 Slot = 0; Slot < MemberCount; ++Slot)
        {
            Members[Slot]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetMaxHealthAttribute(), 1000.f * (Slot + 1));
            Members[Slot]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 100.f);
            Listeners.Add(Members[Slot]->GetGameplayAttributeValueChangeDelegate(
                UAetherBaseAttributeSet::GetHealthAttribute()).AddLambda(
                [this, Slot](const FOnAttributeChangeData& Data)
                {
                    if (Data.NewValue > Data.OldValue)
                        Events.Add({Slot, Clock, Data.NewValue - Data.OldValue});
                }));
        }
    }
    ~FSwitchFixture()
    {
        for (int32 Slot = 0; Slot < Listeners.Num(); ++Slot)
            Members[Slot]->GetGameplayAttributeValueChangeDelegate(
                UAetherBaseAttributeSet::GetHealthAttribute()).Remove(Listeners[Slot]);
    }
    void AdvanceTo(double Time)
    {
        while (Clock + 0.000001 < Time)
        {
            const float Step = static_cast<float>(FMath::Min(0.01, Time - Clock));
            Clock += Step;
            ++GFrameCounter;
            World->GetTimerManager().Tick(Step);
        }
    }
    float HP(int32 Slot) const
    {
        return Members[Slot]->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute());
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldSwitchSnapshotTest,
    "Aether.HealingField.Stage3.SwitchSnapshotAndLifetime", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldSwitchSnapshotTest::RunTest(const FString& Parameters)
{
    HealingFieldTest::FSwitchFixture F;
    if (!TestTrue(TEXT("Three distinct members initialized through public API"), F.bReady)) return false;
    TestEqual(TEXT("Only caster has healing ability"), F.Members[1]->GetActivatableAbilities().Num(), 0);
    TestEqual(TEXT("Third member has no healing ability"), F.Members[2]->GetActivatableAbilities().Num(), 0);
    F.Press();
    auto* Ability = F.Ability();
    auto* Field = Ability->GetSpawnedField();
    if (!TestNotNull(TEXT("Field created"), Field)) return false;
    const FVector OriginalPosition = Field->GetActorLocation();
    TestFalse(TEXT("Cast finished before switching"), F.ASC->FindAbilitySpecFromHandle(F.Handle)->IsActive());
    TestEqual(TEXT("A immediate heal"), F.HP(0), 150.f);
    F.AdvanceTo(0.4);
    TestTrue(TEXT("Switch A to B"), F.Party->RequestSwitchPartySlot(1));
    TestEqual(TEXT("Active ASC is B"), F.Party->GetActiveAbilitySystemComponent(), F.Members[1]);
    TestEqual(TEXT("B has no immediate heal on switch"), F.HP(1), 100.f);
    TestEqual(TEXT("Same field after switch"), Ability->GetSpawnedField(), Field);
    TestTrue(TEXT("Field remains visible"), IsValid(Field) && !Field->IsHidden());
    F.AdvanceTo(1.02);
    TestEqual(TEXT("B receives A snapshot rather than its own max HP"), F.HP(1), 150.f);
    TestEqual(TEXT("Standby A unchanged"), F.HP(0), 150.f);
    F.AdvanceTo(1.4);
    F.Members[0]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetMaxHealthAttribute(), 4000.f);
    TestTrue(TEXT("Switch B to C"), F.Party->RequestSwitchPartySlot(2));
    TestEqual(TEXT("C has no immediate heal"), F.HP(2), 100.f);
    TestEqual(TEXT("Caster remains A"), Field->GetCasterASC(), static_cast<UAbilitySystemComponent*>(F.Members[0]));
    TestEqual(TEXT("Original party retained"), Field->GetSourceParty(), F.Party);
    TestEqual(TEXT("Field stays at original location"), Field->GetActorLocation(), OriginalPosition);
    TestEqual(TEXT("Shared pawn retained"), F.Party->GetFieldCharacterPawn(), F.Pawn);
    F.AdvanceTo(2.02);
    TestEqual(TEXT("C receives original 50 despite A and C max HP"), F.HP(2), 150.f);
    TestEqual(TEXT("Standby B unchanged"), F.HP(1), 150.f);
    F.AdvanceTo(9.9);
    TestTrue(TEXT("Same field alive before original deadline"), IsValid(Field) && Ability->GetSpawnedField() == Field);
    TestEqual(TEXT("Ten healing applications across members"), F.Events.Num(), 10);
    for (int32 Index = 0; Index < F.Events.Num(); ++Index)
    {
        const auto& Event = F.Events[Index];
        TestEqual(TEXT("Only currently active member heals"), Event.Slot, Index < 2 ? Index : 2);
        TestEqual(TEXT("Every application uses snapshot"), Event.Amount, 50.f);
        TestTrue(TEXT("Original schedule preserved"), FMath::Abs(Event.Time - Index) < 0.025);
    }
    F.AdvanceTo(10.02);
    TestFalse(TEXT("Expires at original deadline despite two switches"), IsValid(Field));
    TestNull(TEXT("Caster ability weak reference invalidated"), Ability->GetSpawnedField());
    F.AdvanceTo(12.0);
    TestEqual(TEXT("No healing at or after expiry"), F.Events.Num(), 10);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldSwitchBoundaryTest,
    "Aether.HealingField.Stage3.SwitchTickBoundary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldSwitchBoundaryTest::RunTest(const FString& Parameters)
{
    HealingFieldTest::FSwitchFixture F;
    if (!TestTrue(TEXT("Party ready"), F.bReady)) return false;
    F.Press();
    F.AdvanceTo(0.99);
    TestTrue(TEXT("Switch before first tick"), F.Party->RequestSwitchPartySlot(1));
    F.AdvanceTo(1.02);
    TestEqual(TEXT("New member receives pending tick"), F.HP(1), 150.f);
    TestEqual(TEXT("Previous member did not receive it"), F.HP(0), 150.f);
    F.AdvanceTo(2.02);
    TestEqual(TEXT("B receives tick before switch"), F.HP(1), 200.f);
    TestTrue(TEXT("Switch after tick"), F.Party->RequestSwitchPartySlot(2));
    TestEqual(TEXT("C cannot receive already executed tick"), F.HP(2), 100.f);
    TestEqual(TEXT("No duplicate application on switch"), F.Events.Num(), 3);
    F.AdvanceTo(3.02);
    TestEqual(TEXT("C receives next scheduled tick"), F.HP(2), 150.f);
    TestTrue(TEXT("Return to caster"), F.Party->RequestSwitchPartySlot(0));
    TestEqual(TEXT("Returning does not reactivate ability"), F.Events.Num(), 4);
    F.AdvanceTo(4.02);
    TestEqual(TEXT("Caster heals at next original tick"), F.HP(0), 200.f);
    TestEqual(TEXT("Original ability instance retained"), F.Ability()->GetSpawnedField()->GetCasterASC(), static_cast<UAbilitySystemComponent*>(F.ASC));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldSwitchConditionsTest,
    "Aether.HealingField.Stage3.SwitchRangeAndDeadTarget", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldSwitchConditionsTest::RunTest(const FString& Parameters)
{
    HealingFieldTest::FSwitchFixture F;
    if (!TestTrue(TEXT("Party ready"), F.bReady)) return false;
    F.Press();
    auto* Field = F.Ability()->GetSpawnedField();
    F.AdvanceTo(0.4);
    F.Pawn->SetActorLocation(FVector(600, 0, 88));
    TestTrue(TEXT("Switch outside field"), F.Party->RequestSwitchPartySlot(1));
    F.AdvanceTo(1.02);
    TestEqual(TEXT("Outside B not healed"), F.HP(1), 100.f);
    TestEqual(TEXT("Standby caster not healed instead"), F.HP(0), 150.f);
    F.Pawn->SetActorLocation(FVector(0, 0, 88));
    F.AdvanceTo(1.5);
    TestEqual(TEXT("Reentry does not compensate missed tick"), F.Events.Num(), 1);
    F.AdvanceTo(2.02);
    TestEqual(TEXT("B heals on next scheduled tick"), F.HP(1), 150.f);
    F.Members[1]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 0.f);
    F.AdvanceTo(3.02);
    TestEqual(TEXT("Dead B is not revived"), F.HP(1), 0.f);
    TestEqual(TEXT("Dead target tick produces no healing"), F.Events.Num(), 2);
    TestTrue(TEXT("Living standby caster preserves field"), F.HP(0) > 0.f && IsValid(Field));
    TestTrue(TEXT("Switch from dead B to living C"), F.Party->RequestSwitchPartySlot(2));
    TestEqual(TEXT("No immediate heal after death switch"), F.HP(2), 100.f);
    F.AdvanceTo(4.02);
    TestEqual(TEXT("C resumes healing on original schedule"), F.HP(2), 150.f);
    TestEqual(TEXT("No replacement field"), F.Ability()->GetSpawnedField(), Field);
    TestEqual(TEXT("Standby A still unchanged"), F.HP(0), 150.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldOverflowNumbersTest,
    "Aether.HealingField.Stage4.OverflowNumbers", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldOverflowNumbersTest::RunTest(const FString& Parameters)
{
    struct FCase { const TCHAR* Name; float Before[4]; float After[4]; int32 LivingStandby; };
    const FCase Cases[] = {
        {TEXT("Three equal shares"), {1980,100,100,100}, {2000,126,126,126}, 3},
        {TEXT("Dead member excluded"), {1980,100,100,0}, {2000,140,140,0}, 2},
        {TEXT("Full member still counted"), {1980,2000,100,100}, {2000,2000,126,126}, 3},
        {TEXT("No redistribution of capped share"), {1980,1990,100,100}, {2000,2000,126,126}, 3},
        {TEXT("Full active still distributes"), {2000,100,100,100}, {2000,133,133,133}, 3},
        {TEXT("No living standby"), {1980,0,0,0}, {2000,0,0,0}, 0},
        {TEXT("One living standby"), {1980,100,0,0}, {2000,180,0,0}, 1},
        {TEXT("Fractional field deficit is not reclaimed"), {1998.5f,100,100,100}, {1999.5f,132,132,132}, 3},
        {TEXT("Fractional standby deficit is floored"), {1980,1998.5f,1999.5f,100}, {2000,1999.5f,1999.5f,126}, 3},
        {TEXT("No overflow"), {100,100,100,100}, {200,100,100,100}, 3}
    };
    for (const auto& Case : Cases)
    {
        HealingFieldTest::FSwitchFixture F(4);
        if (!TestTrue(TEXT("Four member party ready"), F.bReady)) return false;
        for (int32 Slot = 0; Slot < 4; ++Slot)
        {
            F.Members[Slot]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetMaxHealthAttribute(), 2000.f);
            F.Members[Slot]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), Case.Before[Slot]);
        }
        TestEqual(FString(Case.Name) + TEXT(" query count"),
            F.Party->GetLivingStandbyAbilitySystemComponents().Num(), Case.LivingStandby);
        F.Press();
        for (int32 Slot = 0; Slot < 4; ++Slot)
            TestEqual(FString::Printf(TEXT("%s slot %d"), Case.Name, Slot), F.HP(Slot), Case.After[Slot]);
    }
    HealingFieldTest::FSwitchFixture Single(1);
    if (!TestTrue(TEXT("Single member party ready"), Single.bReady)) return false;
    Single.Members[0]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 990.f);
    Single.Press();
    TestEqual(TEXT("Single party excess discarded"), Single.HP(0), 1000.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldOverflowSwitchTest,
    "Aether.HealingField.Stage4.SwitchAndTickEligibility", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldOverflowSwitchTest::RunTest(const FString& Parameters)
{
    HealingFieldTest::FSwitchFixture F;
    if (!TestTrue(TEXT("Party ready"), F.bReady)) return false;
    // H=50. After switching to full B, standby A and C each receive 25.
    F.Members[1]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 2000.f);
    F.Press();
    F.AdvanceTo(0.4);
    TestTrue(TEXT("Switch to full B"), F.Party->RequestSwitchPartySlot(1));
    F.AdvanceTo(1.02);
    TestEqual(TEXT("Caster A receives standby share"), F.HP(0), 175.f);
    TestEqual(TEXT("Active B excluded from standby count"), F.HP(1), 2000.f);
    TestEqual(TEXT("C gets equal share"), F.HP(2), 125.f);
    F.Pawn->SetActorLocation(FVector(600,0,88));
    F.AdvanceTo(2.02);
    TestEqual(TEXT("Outside active suppresses standby A"), F.HP(0), 175.f);
    TestEqual(TEXT("Outside active suppresses standby C"), F.HP(2), 125.f);
    F.Pawn->SetActorLocation(FVector(0,0,88));
    F.Members[1]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 0.f);
    F.AdvanceTo(3.02);
    TestEqual(TEXT("Dead active suppresses all distribution"), F.HP(0), 175.f);
    TestEqual(TEXT("Dead active remains dead"), F.HP(1), 0.f);
    TestEqual(TEXT("No standby fallback on dead active"), F.HP(2), 125.f);
    F.Members[2]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 3000.f);
    TestTrue(TEXT("Switch to living C"), F.Party->RequestSwitchPartySlot(2));
    F.AdvanceTo(4.02);
    TestEqual(TEXT("Recomputed denominator excludes dead B"), F.HP(0), 225.f);
    TestEqual(TEXT("Dead B still not revived"), F.HP(1), 0.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldOverflowIsolationTest,
    "Aether.HealingField.Stage4.OtherPartyIsolation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldOverflowIsolationTest::RunTest(const FString& Parameters)
{
    HealingFieldTest::FSwitchFixture F;
    if (!TestTrue(TEXT("Source party ready"), F.bReady)) return false;
    // A second real party in the SAME world, using the exact same character definition.
    auto* OtherPS = F.World->SpawnActor<AAetherPlayerState>();
    auto* Data = NewObject<UAetherInitialPlayerData>(OtherPS);
    Data->CharacterDefinitions.Add(F.Definition);
    FindFProperty<FObjectProperty>(OtherPS->GetClass(), TEXT("InitialPlayerData"))->SetObjectPropertyValue_InContainer(OtherPS, Data);
    auto* OtherPawn = F.World->SpawnActor<AAetherFieldCharacterPawn>();
    OtherPawn->SetActorLocation(FVector(0,0,88));
    auto* OtherParty = OtherPS->GetPartyComponent();
    OtherParty->InitializeFieldPawn(OtherPawn);
    OtherParty->InitializeParty({F.Definition->GetPrimaryAssetId()});
    auto* OtherASC = OtherParty->GetActiveAbilitySystemComponent();
    if (!TestNotNull(TEXT("Other party active ASC"), OtherASC)) return false;
    OtherASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 10.f);
    F.Members[0]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 1000.f);
    F.Press();
    F.AdvanceTo(1.02);
    TestEqual(TEXT("Own standby B receives two shares"), F.HP(1), 150.f);
    TestEqual(TEXT("Own standby C receives two shares"), F.HP(2), 150.f);
    TestEqual(TEXT("Other party at same location receives nothing"),
        OtherASC->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute()), 10.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldOverflowMutationTest,
    "Aether.HealingField.Stage4.RecipientDiesDuringApplication", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldOverflowMutationTest::RunTest(const FString& Parameters)
{
    HealingFieldTest::FSwitchFixture F(4);
    if (!TestTrue(TEXT("Party ready"), F.bReady)) return false;
    F.Members[0]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetMaxHealthAttribute(), 2000.f);
    F.Members[0]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 1980.f);
    // Field heal callback kills a recipient AFTER the list/share were calculated.
    const auto Listener = F.Members[0]->GetGameplayAttributeValueChangeDelegate(
        UAetherBaseAttributeSet::GetHealthAttribute()).AddLambda([&](const FOnAttributeChangeData& Change)
        {
            if (Change.NewValue > Change.OldValue)
                F.Members[1]->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 0.f);
        });
    F.Press();
    F.Members[0]->GetGameplayAttributeValueChangeDelegate(UAetherBaseAttributeSet::GetHealthAttribute()).Remove(Listener);
    TestEqual(TEXT("Dead recipient not revived"), F.HP(1), 0.f);
    TestEqual(TEXT("Lost share not redistributed to C"), F.HP(2), 126.f);
    TestEqual(TEXT("Lost share not redistributed to D"), F.HP(3), 126.f);
    F.AdvanceTo(1.02);
    TestEqual(TEXT("Next tick uses new two-member denominator"), F.HP(2), 176.f);
    TestEqual(TEXT("Next tick D receives 50"), F.HP(3), 176.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldCasterDeathTest,
    "Aether.HealingField.Stage5.CasterDeathActiveAndStandby", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldCasterDeathTest::RunTest(const FString& Parameters)
{
    for (bool bStandby : {false, true})
    {
        HealingFieldTest::FSwitchFixture F;
        if (!TestTrue(TEXT("Party ready"), F.bReady)) return false;
        F.Press();
        auto* Field = F.Ability()->GetSpawnedField();
        if (!TestNotNull(TEXT("Field started"), Field)) return false;
        F.AdvanceTo(0.4);
        if (bStandby) TestTrue(TEXT("Caster enters standby"), F.Party->RequestSwitchPartySlot(1));
        // Actual damage GE -> IncomingDamage -> AttributeSet::PostGameplayEffectExecute -> Health.
        auto* Damage = NewObject<UGameplayEffect>(F.World);
        Damage->DurationPolicy = EGameplayEffectDurationType::Instant;
        FGameplayModifierInfo Modifier;
        Modifier.Attribute = UAetherBaseAttributeSet::GetIncomingDamageAttribute();
        Modifier.ModifierOp = EGameplayModOp::Additive;
        Modifier.ModifierMagnitude = FScalableFloat(10000.f);
        Damage->Modifiers.Add(Modifier);
        FGameplayEffectSpec Spec(Damage, F.ASC->MakeEffectContext(), 1.f);
        F.ASC->ApplyGameplayEffectSpecToSelf(Spec);
        // No timer/world advance between damage and these assertions.
        TestEqual(TEXT("Damage killed original caster"), F.HP(0), 0.f);
        TestFalse(TEXT("Field destroyed synchronously between ticks"), IsValid(Field));
        TestNull(TEXT("Ability no longer exposes field"), F.Ability()->GetSpawnedField());
        TestFalse(TEXT("Death listener removed"), F.ASC->GetGameplayAttributeValueChangeDelegate(
            UAetherBaseAttributeSet::GetHealthAttribute()).IsBoundToObject(Field));
        const int32 Count = F.Events.Num();
        const float BHealth = F.HP(1), CHealth = F.HP(2);
        F.AdvanceTo(12.0);
        TestEqual(TEXT("No later healing event"), F.Events.Num(), Count);
        TestEqual(TEXT("B unchanged after caster death"), F.HP(1), BHealth);
        TestEqual(TEXT("C unchanged after caster death"), F.HP(2), CHealth);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldDeathCleanupTest,
    "Aether.HealingField.Stage5.DelegateCleanup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldDeathCleanupTest::RunTest(const FString& Parameters)
{
    HealingFieldTest::FFixture F;
    auto& Changed = F.ASC->GetGameplayAttributeValueChangeDelegate(UAetherBaseAttributeSet::GetHealthAttribute());
    F.Press();
    auto* Old = F.Ability()->GetSpawnedField();
    if (!TestNotNull(TEXT("First field"), Old)) return false;
    TestTrue(TEXT("First field watches caster"), Changed.IsBoundToObject(Old));
    F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 1.f);
    TestTrue(TEXT("Nonlethal HP change preserves field"), IsValid(Old));
    F.Press();
    auto* Replacement = F.Ability()->GetSpawnedField();
    if (!TestNotNull(TEXT("Replacement"), Replacement)) return false;
    TestFalse(TEXT("Replacement removes old listener"), Changed.IsBoundToObject(Old));
    TestTrue(TEXT("Replacement has own listener"), Changed.IsBoundToObject(Replacement));
    F.Advance(10.1f);
    TestFalse(TEXT("Natural expiry removes listener"), Changed.IsBoundToObject(Replacement));
    F.Press();
    auto* Removed = F.Ability()->GetSpawnedField();
    if (!TestNotNull(TEXT("Recast after expiry"), Removed)) return false;
    Removed->Destroy();
    TestFalse(TEXT("External destroy removes listener"), Changed.IsBoundToObject(Removed));
    F.Press();
    auto* FinalField = F.Ability()->GetSpawnedField();
    F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), -1.f);
    TestFalse(TEXT("Negative health also destroys field"), IsValid(FinalField));
    TestFalse(TEXT("Final death removes listener"), Changed.IsBoundToObject(FinalField));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAetherHealingFieldDeathDuringHealTest,
    "Aether.HealingField.Stage5.DeathDuringHealing", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAetherHealingFieldDeathDuringHealTest::RunTest(const FString& Parameters)
{
    for (bool bImmediate : {true, false})
    {
        HealingFieldTest::FSwitchFixture F;
        if (!TestTrue(TEXT("Party ready"), F.bReady)) return false;
        if (!bImmediate)
        {
            F.Press();
            F.AdvanceTo(0.4);
            TestTrue(TEXT("Switch to B"), F.Party->RequestSwitchPartySlot(1));
        }
        auto* TriggerASC = bImmediate ? F.Members[0] : F.Members[1];
        const float Maximum = TriggerASC->GetNumericAttribute(UAetherBaseAttributeSet::GetMaxHealthAttribute());
        TriggerASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), Maximum - 1.f);
        const float CBefore = F.HP(2);
        const auto Kill = TriggerASC->GetGameplayAttributeValueChangeDelegate(
            UAetherBaseAttributeSet::GetHealthAttribute()).AddLambda([&](const FOnAttributeChangeData& Change)
            {
                if (Change.NewValue > Change.OldValue)
                    F.ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 0.f);
            });
        if (bImmediate) F.Press(); else F.AdvanceTo(1.02);
        TriggerASC->GetGameplayAttributeValueChangeDelegate(UAetherBaseAttributeSet::GetHealthAttribute()).Remove(Kill);
        TestEqual(TEXT("Callback killed caster"), F.HP(0), 0.f);
        TestNull(TEXT("Field ends even during a healing callback"), F.Ability()->GetSpawnedField());
        TestEqual(TEXT("Remaining overflow not applied after death"), F.HP(2), CBefore);
        const int32 Count = F.Events.Num();
        F.AdvanceTo(12.0);
        TestEqual(TEXT("Callback did not rearm healing timer"), F.Events.Num(), Count);
    }
    return true;
}

#endif
