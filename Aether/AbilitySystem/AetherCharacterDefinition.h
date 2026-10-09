// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AetherCharacterDefinition.generated.h"

class AAetherCharacterBase;
class UAetherInputConfig;
class UAetherAbilitySet;


USTRUCT(BlueprintType)
struct AETHER_API FAetherOwnedCharacterData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPrimaryAssetId CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SavedHealth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SavedEnergy = 0.0f;
};



namespace AetherAssetBundles
{
	inline const FName UI(TEXT("UI"));
	inline const FName Display(TEXT("Display"));
	inline const FName Gameplay(TEXT("Gameplay"));
}

UCLASS()
class AETHER_API UAetherCharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(
			TEXT("CharacterDefinition"),
			GetFName());
	}

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Character")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Character", meta = (Categories = "Element"))
	FGameplayTag Element;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Character", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> Portrait;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Mesh", meta = (AssetBundles = "Display"))
	TSoftObjectPtr<USkeletalMesh> DisplayMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Mesh", meta = (AssetBundles = "Display"))
	TSoftClassPtr<UAnimInstance> DisplayAnimInstanceClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Abilities", meta = (AssetBundles = "Gameplay"))
	TArray<TSoftObjectPtr<UAetherAbilitySet>> AbilitySets;
};
