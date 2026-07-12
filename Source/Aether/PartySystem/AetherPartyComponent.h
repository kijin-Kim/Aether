// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherPartyComponent.generated.h"


class UAetherAbilitySystemComponent;
class AAetherFieldCharacterPawn;
class AAetherPartyMemberStateActor;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AETHER_API UAetherPartyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherPartyComponent();
	virtual void BeginPlay() override;
	void InitializeParty(const TArray<FName>& CharacterIds);
	void SpawnAndSetupCharacters(const TArray<FName>& CharacterIds);
	
	bool RequestSwitchPartySlot(int32 SlotIndex);

	
	AAetherFieldCharacterPawn* GetFieldCharacterPawn() const { return FieldCharacterPawn; }
	UAetherAbilitySystemComponent* GetActiveAbilitySystemComponent() const;

private:
	UPROPERTY()
	TObjectPtr<AAetherFieldCharacterPawn> FieldCharacterPawn;
	uint32 ActiveSlotIndex = 0;


	UPROPERTY(EditDefaultsOnly, Category = "Aether|Party")
	TArray<FName> PartyCharacterIds;
	UPROPERTY()
	TArray<TObjectPtr<AAetherPartyMemberStateActor>> PartyMemberStateActors;
};
