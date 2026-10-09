#include "AetherPlayerState.h"

#include "DisplayDebugHelpers.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/PartySystem/AetherPartyComponent.h"


AAetherPlayerState::AAetherPlayerState()
{
	bReplicates = false;

	PartyASC = CreateDefaultSubobject<UAetherAbilitySystemComponent>(TEXT("PartyASC"));
	PartyComponent = CreateDefaultSubobject<UAetherPartyComponent>(TEXT("PartyComponent"));
}

void AAetherPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (!InitialPlayerData)
	{
		UE_LOG(LogTemp, Warning, TEXT("AetherPlayerState skipped party initialization: InitialPlayerData is not set."));
		OwnedRoster.Reset();
		bPlayerDataInitialized = true;
		OnPlayerDataInitialized.Broadcast();
		return;
	}

	OwnedRoster = InitialPlayerData->InitialRoster;
	PartyComponent->InitializeParty(InitialPlayerData->InitialParty);
	bPlayerDataInitialized = true;
	OnPlayerDataInitialized.Broadcast();
}

void AAetherPlayerState::CallOrRegisterPlayerDataInitialized(FOnAetherPlayerDataInitializedCallback Callback)
{
	if (!Callback.IsBound())
	{
		return;
	}

	if (bPlayerDataInitialized)
	{
		Callback.Execute();
		return;
	}

	OnPlayerDataInitialized.AddUnique(Callback);
}

void AAetherPlayerState::ProcessInputs()
{
	PartyASC->ProcessInputs();
	UAetherAbilitySystemComponent* ActiveASC = GetActiveAbilitySystemComponent();
	if (ActiveASC)
	{
		ActiveASC->ProcessInputs();
	}
}

bool AAetherPlayerState::RequestReplacePartyMember(int32 SlotIndex, FPrimaryAssetId NewCharacterId,
                                                   FOnAetherPartyMemberReplaceCompleted OnCompleted)
{
	const bool bOwnsCharacter = OwnedRoster.ContainsByPredicate(
		[&NewCharacterId](const FAetherOwnedCharacterData& OwnedCharacter)
		{
			return OwnedCharacter.CharacterId == NewCharacterId;
		});

	if (!bPlayerDataInitialized || !bOwnsCharacter || !PartyComponent)
	{
		OnCompleted.ExecuteIfBound(false);
		return false;
	}

	return PartyComponent->RequestReplacePartyMember(SlotIndex, NewCharacterId, MoveTemp(OnCompleted));
}

void AAetherPlayerState::DisplayDebug(UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL,
                                      float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	if (DebugDisplay.IsDisplayOn(TEXT("AbilitySystem")))
	{
		if (UAetherAbilitySystemComponent* PartyAbilitySystem = GetPartyASC())
		{
			PartyAbilitySystem->DisplayDebug(Canvas, DebugDisplay, YL, YPos);
		}

		if (UAetherAbilitySystemComponent* ActiveAbilitySystem = GetActiveAbilitySystemComponent())
		{
			ActiveAbilitySystem->DisplayDebug(Canvas, DebugDisplay, YL, YPos);
		}
	}
}


UAetherAbilitySystemComponent* AAetherPlayerState::GetActiveAbilitySystemComponent() const
{
	return PartyComponent->GetActiveAbilitySystemComponent();
}
