// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AetherCharacterData.generated.h"

class AAetherCharacterBase;
class UAetherInputConfig;
class UAetherAbilitySet;

UCLASS()
class AETHER_API UAetherCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("Character", CharacterId);
	}

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Character")
	FName CharacterId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Character", meta = (Categories = "Element"))
	FGameplayTag Element;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Character")
	TSoftClassPtr<AAetherCharacterBase> CharacterClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Mesh")
	TSoftObjectPtr<USkeletalMesh> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Mesh")
	TSoftClassPtr<UAnimInstance> AnimInstanceClass;

	// 플레이어 캐릭터에만 사용. AI는 비워두면 됨.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Input")
	TSoftObjectPtr<UAetherInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aether|Abilities")
	TArray<TSoftObjectPtr<UAetherAbilitySet>> AbilitySets;
};
