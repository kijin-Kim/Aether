// AetherPlayerState.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "AetherPlayerState.generated.h"


class AAetherCharacter;
/**
 *
 *
 */
UCLASS()
class AETHER_API AAetherPlayerState : public APlayerState
{
    GENERATED_BODY()
    
public:
    void AuthInitializeParty(AAetherCharacter* CurrentCharacter);
    void AuthSwitchPartySlot(int32 SlotIndex);
    

private:
    UPROPERTY()
    TArray<TObjectPtr<AAetherCharacter>> PartyCharacters;
    UPROPERTY()
    int32 ActiveSlotIndex = INDEX_NONE;
};