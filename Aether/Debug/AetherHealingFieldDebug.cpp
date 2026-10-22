#include "AetherHealingFieldDebug.h"
#if WITH_EDITOR
#include "Aether/Aether.h"
#include "Aether/AetherGameplayTags.h"
#include "Aether/Player/AetherPlayerController.h"
#include "Aether/Player/AetherPlayerState.h"
#include "Aether/PartySystem/AetherPartyComponent.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AttributeSet/AetherBaseAttributeSet.h"
#include "Aether/AbilitySystem/Actors/AetherHealingFieldActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "String/LexFromString.h"

namespace AetherHealingFieldDebug
{
namespace
{
TWeakObjectPtr<UWorld> WatchedWorld;
void Print(const FString& Text)
{
	UE_LOG(LogAether, Display, TEXT("[HealTest] %s"), *Text);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, Text);
	}
}
FString PartyStatus(UAetherPartyComponent* Party)
{
	FString Text;
	const auto IDs = Party->GetPartyCharacterIds();
	for (int32 Slot = 0; Slot < IDs.Num(); ++Slot)
	{
		if (auto* ASC = Party->GetMemberAbilitySystemComponent(Slot))
		{
			Text += FString::Printf(TEXT(" [%d%s %s HP=%.2f/%.2f]"), Slot + 1, Slot == Party->GetActiveSlotIndex() ? TEXT("*") : TEXT(""), *IDs[Slot].PrimaryAssetName.ToString(), ASC->GetNumericAttribute(UAetherBaseAttributeSet::GetHealthAttribute()), ASC->GetNumericAttribute(UAetherBaseAttributeSet::GetMaxHealthAttribute()));
		}
	}
	return Text;
}
void StopFields(UWorld* World, UAetherPartyComponent* Party)
{
	TArray<AAetherHealingFieldActor*> Fields;
	for (TActorIterator<AAetherHealingFieldActor> It(World); It; ++It)
	{
		if (It->GetSourceParty() == Party)
		{
			Fields.Add(*It);
		}
	}
	for (auto* Field : Fields)
	{
		Field->Destroy();
	}
}
} // namespace

void Report(const AAetherHealingFieldActor* Field, const TCHAR* Event)
{
	if (!Field || WatchedWorld.Get() != Field->GetWorld())
	{
		return;
	}
	if (auto* Party = Field->GetSourceParty())
	{
		Print(FString::Printf(TEXT("%s %s t=%.2f count=%d/%d%s"), Event, *Field->GetName(), Field->GetWorld()->GetTimeSeconds(), Field->GetExecutedHealingTicks(), Field->GetTotalHealingTicks(), *PartyStatus(Party)));
	}
}

bool Execute(UWorld* World, const TArray<FString>& Args)
{
	if (!World || (World->WorldType != EWorldType::PIE && !(GIsAutomationTesting && World->WorldType == EWorldType::Game)))
	{
		Print(TEXT("Only available in PIE."));
		return false;
	}
	auto* PC = Cast<AAetherPlayerController>(World->GetFirstPlayerController());
	auto* PS = PC ? PC->GetPlayerState<AAetherPlayerState>() : nullptr;
	auto* Party = PS ? PS->GetPartyComponent() : nullptr;
	if (!Party || !Party->GetActiveAbilitySystemComponent())
	{
		Print(TEXT("No initialized local player party."));
		return false;
	}
	const FString Op = Args.IsEmpty() ? TEXT("help") : Args[0].ToLower();
	if (Op == TEXT("help"))
	{
		Print(TEXT("aether.HealTest: status | watch 0/1 | hp SLOT HP MAXHP | switch SLOT | cast | stop | preset basic/overflow/full | hitch SECONDS(0.1..3). Slots are 1-based."));
		return true;
	}
	if (Op == TEXT("status") && Args.Num() == 1)
	{
		Print(PartyStatus(Party));
		int32 Count = 0;
		for (TActorIterator<AAetherHealingFieldActor> It(World); It; ++It)
		{
			if (It->GetSourceParty() == Party)
			{
				++Count;
				Print(FString::Printf(TEXT("Field=%s judgement=%d/%d position=%s"), *It->GetName(), It->GetExecutedHealingTicks(), It->GetTotalHealingTicks(), *It->GetActorLocation().ToString()));
			}
		}
		Print(FString::Printf(TEXT("Own fields=%d"), Count));
		return true;
	}
	if (Op == TEXT("watch") && Args.Num() == 2 && (Args[1] == TEXT("0") || Args[1] == TEXT("1")))
	{
		WatchedWorld = Args[1] == TEXT("1") ? World : nullptr;
		Print(TEXT("Watch updated. Tick/end records go to Output Log and screen."));
		return true;
	}
	if ((Op == TEXT("hp") && Args.Num() == 4) || (Op == TEXT("switch") && Args.Num() == 2))
	{
		int32 Slot = 0;
		if (!LexTryParseString(Slot, *Args[1]))
		{
			return false;
		}
		auto* ASC = Party->GetMemberAbilitySystemComponent(Slot - 1);
		if (!ASC)
		{
			Print(TEXT("Invalid slot (use 1..party size)."));
			return false;
		}
		if (Op == TEXT("switch"))
		{
			const bool Result = Party->RequestSwitchPartySlot(Slot - 1);
			Print(Result ? PartyStatus(Party) : TEXT("Switch rejected."));
			return Result;
		}
		float HP = 0, MaxHP = 0;
		if (!LexTryParseString(HP, *Args[2]) || !LexTryParseString(MaxHP, *Args[3]) || !FMath::IsFinite(HP) || !FMath::IsFinite(MaxHP) || MaxHP <= 0 || HP < 0 || HP > MaxHP)
		{
			Print(TEXT("Require finite 0 <= HP <= MAXHP and MAXHP > 0."));
			return false;
		}
		ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetMaxHealthAttribute(), MaxHP);
		ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), HP);
		Print(PartyStatus(Party));
		return true;
	}
	if (Op == TEXT("cast") && Args.Num() == 1)
	{
		PC->AbilityInputPressed(AetherGameplayTags::InputTag_HealingField);
		PS->ProcessInputs();
		PC->AbilityInputReleased(AetherGameplayTags::InputTag_HealingField);
		PS->ProcessInputs();
		Print(TEXT("Cast input requested; use status to check success. This does not test the physical E key."));
		return true;
	}
	if (Op == TEXT("stop") && Args.Num() == 1)
	{
		StopFields(World, Party);
		Print(TEXT("Own party fields removed."));
		return true;
	}
	if (Op == TEXT("preset") && Args.Num() == 2 && (Args[1] == TEXT("basic") || Args[1] == TEXT("overflow") || Args[1] == TEXT("full")))
	{
		StopFields(World, Party);
		for (int32 Slot = 0; Slot < Party->GetPartyCharacterIds().Num(); ++Slot)
		{
			if (auto* ASC = Party->GetMemberAbilitySystemComponent(Slot))
			{
				ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetMaxHealthAttribute(), 2000.f);
				const float HP = Args[1] == TEXT("full") ? 2000.f : Args[1] == TEXT("overflow") && Slot == Party->GetActiveSlotIndex() ? 1980.f : 100.f;
				ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), HP);
			}
		}
		WatchedWorld = World;
		Print(TEXT("Preset set; existing field cleared; MaxHP=2000 for all. No automatic cast. PIE runtime only."));
		Print(PartyStatus(Party));
		return true;
	}
	if (Op == TEXT("hitch") && Args.Num() == 2)
	{
		float Seconds = 0;
		if (!LexTryParseString(Seconds, *Args[1]) || !FMath::IsFinite(Seconds) || Seconds < 0.1f || Seconds > 3.f)
		{
			return false;
		}
		Print(TEXT("Intentional game-thread stall; editor will resume automatically."));
		FPlatformProcess::Sleep(Seconds);
		return true;
	}
	Print(TEXT("Invalid arguments. Run aether.HealTest help."));
	return false;
}
} // namespace AetherHealingFieldDebug

static FAutoConsoleCommandWithWorldAndArgs HealingFieldTestCommand(TEXT("aether.HealTest"), TEXT("PIE healing field test helpers. Use help."),
                                                                   FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
                                                                       [](const TArray<FString>& Args, UWorld* World)
                                                                       {
	                                                                       AetherHealingFieldDebug::Execute(World, Args);
                                                                       }));
#endif
