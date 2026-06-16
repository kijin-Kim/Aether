#include "AetherPlayerState.h"

#include "Aether/Aether.h"
#include "Aether/Character/AetherCharacter.h"
#include "AetherPartyComponent.h"
#include "AetherPlayerController.h"
#include "GameFramework/PlayerController.h"


void AAetherPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (UAetherPartyComponent* PartyComponent = GetPartyComponent())
	{
		PartyComponent->OnActivePartySlotChanged.AddUniqueDynamic(this, &AAetherPlayerState::HandleActivePartySlotChanged);
	}
}

void AAetherPlayerState::SpawnAndSetupCharacter(const TArray<FName>& CharacterIds)
{
	if (UAetherPartyComponent* PartyComponent = GetPartyComponent())
	{
		PartyComponent->InitializeParty(CharacterIds);
	}
}

void AAetherPlayerState::SwitchPartySlot(int32 SlotIndex)
{
	if (UAetherPartyComponent* PartyComponent = GetPartyComponent())
	{
		PartyComponent->RequestSwitchPartySlot(SlotIndex);
	}
}

AAetherCharacter* AAetherPlayerState::GetActivePartyCharacter() const
{
	if (const UAetherPartyComponent* PartyComponent = GetPartyComponent())
	{
		return PartyComponent->GetActivePartyCharacter();
	}

	return nullptr;
}

void AAetherPlayerState::HandleActivePartySlotChanged(int32 PreviousSlotIndex, int32 NewSlotIndex, AAetherCharacter* PreviousCharacter, AAetherCharacter* NewCharacter)
{
	OnActivePartySlotChanged.Broadcast(PreviousSlotIndex, NewSlotIndex, PreviousCharacter, NewCharacter);
}

UAetherPartyComponent* AAetherPlayerState::GetPartyComponent() const
{
	const AAetherPlayerController* AetherPlayerController = Cast<AAetherPlayerController>(GetPlayerController());
	if (!AetherPlayerController)
	{
		return nullptr;
	}

	return AetherPlayerController->GetPartyComponent();
}
