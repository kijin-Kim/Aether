// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AetherAbilitySet.generated.h"


class UAetherAbilitySystemComponent;
class UAetherGameplayAbility;

USTRUCT(BlueprintType)
struct FAetherAbilitySet_GameplayAbility
{
	GENERATED_BODY()
	
public:
	FAetherAbilitySet_GameplayAbility();
	
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAetherGameplayAbility> Ability;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 1))
	int32 Level;
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;
};



/**
 * 
 */
UCLASS(Const)
class AETHER_API UAetherAbilitySet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	void InitializeAbilitySystem(UAetherAbilitySystemComponent* AetherASC, UObject* SourceObject = nullptr) const;
	
public:
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = Ability))
	TArray<FAetherAbilitySet_GameplayAbility> Abilities;
	
};
