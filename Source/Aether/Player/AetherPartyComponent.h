// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherPartyComponent.generated.h"

class AAetherCharacter;
class AAetherPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FAetherPartySlotChangedDelegate,
	int32, PreviousSlotIndex,
	int32, NewSlotIndex,
	AAetherCharacter*, PreviousCharacter,
	AAetherCharacter*, NewCharacter);

UCLASS(ClassGroup=(Aether), meta=(BlueprintSpawnableComponent))
class AETHER_API UAetherPartyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherPartyComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Aether|Party")
	void InitializeParty(const TArray<FName>& CharacterIds);

	UFUNCTION(BlueprintCallable, Category = "Aether|Party")
	bool RequestSwitchPartySlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Aether|Party")
	AAetherCharacter* GetActivePartyCharacter() const;

	UFUNCTION(BlueprintPure, Category = "Aether|Party")
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	const TArray<TObjectPtr<AAetherCharacter>>& GetPartyCharacters() const { return PartyCharacters; }

	UPROPERTY(BlueprintAssignable, Category = "Aether|Party")
	FAetherPartySlotChangedDelegate OnActivePartySlotChanged;

private:
	void SpawnAndSetupCharacters(const TArray<FName>& CharacterIds);
	AAetherPlayerController* GetOwningAetherPlayerController() const;
	AAetherCharacter* GetPartyCharacterAtSlot(int32 SlotIndex) const;
	bool CanSwitchFromCurrentCharacter() const;
	void CopyPartySwapState(const AAetherCharacter* PreviousCharacter, AAetherCharacter* NewCharacter) const;
	void ClearSwapInputs(AAetherCharacter* Character) const;
	void CancelSwapAbilities(AAetherCharacter* Character) const;
	void ClearPartyRoster(bool bDestroyExistingCharacters);

	UPROPERTY(EditDefaultsOnly, Category = "Aether|Party")
	bool bAutoInitializeInitialParty = true;

	UPROPERTY(EditDefaultsOnly, Category = "Aether|Party")
	TArray<FName> InitialPartyCharacterIds;

	UPROPERTY()
	TArray<TObjectPtr<AAetherCharacter>> PartyCharacters;

	UPROPERTY()
	int32 ActiveSlotIndex = INDEX_NONE;

	bool bIsSwitching = false;
};
