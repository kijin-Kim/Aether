#include "AetherPlayerState.h"

#include "AetherCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"


void AAetherPlayerState::AuthInitializeParty(AAetherCharacter* CurrentCharacter)
{
	check(HasAuthority());
	
	if (PartyCharacters.Num() > 0)
	{
		return;
	}
	
	PartyCharacters.SetNum(4);
	PartyCharacters[0] = CurrentCharacter;
	for (int32 i = 1; i < 4; ++i)
	{
		FTransform SpawnTransform = CurrentCharacter->GetActorTransform();
		AAetherCharacter* NewCharacter = GetWorld()->SpawnActor<AAetherCharacter>(CurrentCharacter->GetClass(), SpawnTransform);
		if (NewCharacter)
		{
			PartyCharacters[i] = NewCharacter;
		}
	}

	for (AAetherCharacter* Character : PartyCharacters)
	{
		if (Character)
		{
			Character->SetOnField(false);
		}
	}

	ActiveSlotIndex = 0;
}

void AAetherPlayerState::AuthSwitchPartySlot(int32 SlotIndex)
{
	check(HasAuthority());
	if (SlotIndex <= INDEX_NONE || SlotIndex >= PartyCharacters.Num())
	{
		return;
	}

	if (SlotIndex == ActiveSlotIndex)
	{
		return;
	}

	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}

	
	if (PartyCharacters.IsValidIndex(SlotIndex) && PartyCharacters[SlotIndex])
	{
		PC->Possess(PartyCharacters[SlotIndex]);
		if (AAetherCharacter* OldCharacter = ActiveSlotIndex != INDEX_NONE ? PartyCharacters[ActiveSlotIndex] : nullptr)
		{
			PartyCharacters[SlotIndex]->SetActorTransform(OldCharacter->GetActorTransform());	
		}
		ActiveSlotIndex = SlotIndex;
	}
}
