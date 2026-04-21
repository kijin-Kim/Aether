// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AetherAbilitySet.generated.h"


class UAetherAbilitySystemComponent;
class UAetherGameplayAbility;

USTRUCT()
struct FAetherAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	void ClearAbilitySystem(UAetherAbilitySystemComponent* AetherASC);
	

public:
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
	TArray<FActiveGameplayEffectHandle> EffectSpecHandles;
};


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


USTRUCT(BlueprintType)
struct FAetherAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> Effect;

	UPROPERTY(EditDefaultsOnly)
	float Level = 1.0f;
};


/**
 * 
 */
UCLASS(Const)
class AETHER_API UAetherAbilitySet : public UDataAsset
{
	GENERATED_BODY()

public:
	void InitializeAbilitySystem(UAetherAbilitySystemComponent* AetherASC, FAetherAbilitySet_GrantedHandles& OutGrantedHandles, UObject* SourceObject = nullptr) const;

public:
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = Ability))
	TArray<FAetherAbilitySet_GameplayAbility> Abilities;
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = Effect))
	TArray<FAetherAbilitySet_GameplayEffect> Effects;
};
