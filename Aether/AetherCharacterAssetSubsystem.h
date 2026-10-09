// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AetherCharacterAssetSubsystem.generated.h"

class UAetherCharacterDefinition;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCharacterDefinitionLoaded, UAetherCharacterDefinition*, CharacterDefinition);

UCLASS()
class AETHER_API UAetherCharacterAssetSubsystem
	: public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void LoadCharacterUI(const FPrimaryAssetId& CharacterId, FOnCharacterDefinitionLoaded OnLoaded);
	UFUNCTION(BlueprintCallable)
	void LoadCharacterDisplay(const FPrimaryAssetId& CharacterId, FOnCharacterDefinitionLoaded OnLoaded);
	UFUNCTION(BlueprintCallable)
	void LoadCharacterGameplay(const FPrimaryAssetId& CharacterId, FOnCharacterDefinitionLoaded OnLoaded);
	UFUNCTION(BlueprintCallable)
	void LoadCharacterGameplayAndDisplay(const FPrimaryAssetId& CharacterId, FOnCharacterDefinitionLoaded OnLoaded);
	UFUNCTION(BlueprintCallable)
	UAetherCharacterDefinition* GetLoadedCharacterDefinition(const FPrimaryAssetId& CharacterId) const;

private:
	void LoadCharacter(const FPrimaryAssetId& CharacterId, TArray<FName> BundleNames,
	                   FOnCharacterDefinitionLoaded OnLoaded);
	void HandleCharacterLoaded(uint64 RequestId, FPrimaryAssetId CharacterId,
	                           FOnCharacterDefinitionLoaded OnLoaded);

	uint64 NextLoadRequestId = 0;
	TMap<uint64, TSharedPtr<FStreamableHandle>> ActiveLoadHandles;
};
