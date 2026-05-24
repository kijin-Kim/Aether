// AetherPlayerState.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "AetherPlayerState.generated.h"

class AAetherCharacter;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FAetherActivePartySlotChangedDelegate,
	int32, PreviousSlotIndex,
	int32, NewSlotIndex,
	AAetherCharacter*, PreviousCharacter,
	AAetherCharacter*, NewCharacter);

/**
 *
 *
 */
UCLASS()
class AETHER_API AAetherPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	void SpawnAndSetupCharacter(const TArray<FName>& CharacterIds);
	void AuthSwitchPartySlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Aether|Party")
	AAetherCharacter* GetActivePartyCharacter() const;

	UPROPERTY(BlueprintAssignable, Category = "Aether|Party")
	FAetherActivePartySlotChangedDelegate OnActivePartySlotChanged;

private:
	bool CanSwitchPartySlot(int32 SlotIndex, APlayerController*& OutPlayerController, AAetherCharacter*& OutNewCharacter) const;
	void CopyPartySwapState(const AAetherCharacter* PreviousCharacter, AAetherCharacter* NewCharacter) const;
	void RefreshPartyControl(APlayerController* PlayerController, AAetherCharacter* PreviousCharacter, AAetherCharacter* NewCharacter) const;

	UPROPERTY()
	TArray<TObjectPtr<AAetherCharacter>> PartyCharacters;

	UPROPERTY()
	int32 ActiveSlotIndex = INDEX_NONE;
};
