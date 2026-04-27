// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AetherCharacterDatabase.generated.h"

class UAetherCharacterData;
/**
 * 
 */
UCLASS()
class AETHER_API UAetherCharacterDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void OnCharacterIndexLoaded();
	UAetherCharacterData* GetCharacterByID(FName CharacterId);
	void LoadCharacterDataAsync(const TArray<FName>& CharacterIds, FStreamableDelegate OnAllLoaded);
	
private:
	UPROPERTY()
	TMap<FName, TObjectPtr<UAetherCharacterData>> LoadedCharacters;
	TArray<TSharedPtr<FStreamableHandle>> ActiveLoadHandles;
};
