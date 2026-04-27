// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AttributeSetCommon.h"
#include "AetherHeroAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class AETHER_API UAetherHeroAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	ATTRIBUTE_ACCESSORS(UAetherHeroAttributeSet, CritRate)
	ATTRIBUTE_ACCESSORS(UAetherHeroAttributeSet, CritDamage)
	ATTRIBUTE_ACCESSORS(UAetherHeroAttributeSet, EnergyRecharge)
	ATTRIBUTE_ACCESSORS(UAetherHeroAttributeSet, ElementalMastery)
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats")
	FGameplayAttributeData CritRate;
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats")
	FGameplayAttributeData CritDamage;
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats")
	FGameplayAttributeData EnergyRecharge;
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats")
	FGameplayAttributeData ElementalMastery;
};
