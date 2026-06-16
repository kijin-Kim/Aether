// AetherPlayerState.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "AetherPlayerState.generated.h"

class AAetherCharacter;
class APlayerController;
class UAetherPartyComponent;

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
	void SwitchPartySlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Aether|Party")
	AAetherCharacter* GetActivePartyCharacter() const;

	UPROPERTY(BlueprintAssignable, Category = "Aether|Party")
	FAetherActivePartySlotChangedDelegate OnActivePartySlotChanged;

private:
	UFUNCTION()
	void HandleActivePartySlotChanged(int32 PreviousSlotIndex, int32 NewSlotIndex, AAetherCharacter* PreviousCharacter, AAetherCharacter* NewCharacter);

	UAetherPartyComponent* GetPartyComponent() const;
};
