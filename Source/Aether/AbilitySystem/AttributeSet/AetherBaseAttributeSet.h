// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AttributeSetCommon.h"
#include "AetherBaseAttributeSet.generated.h"




class FLifetimeProperty;
/**
 * 
 */
UCLASS()
class AETHER_API UAetherBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAetherBaseAttributeSet();
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, Level)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, Health)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, MaxHealth)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, Atk)

	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, PhysicalDamageBonus)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, PyroDamageBonus)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, HydroDamageBonus)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, ElectroDamageBonus)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, AnemoDamageBonus)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, CryoDamageBonus)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, GeoDamageBonus)
	
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, PhysicalRes)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, PyroRes)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, HydroRes)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, ElectroRes)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, AnemoRes)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, CryoRes)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, GeoRes)

	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, PyroAura)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, HydroAura)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, ElectroAura)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, AnemoAura)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, CryoAura)
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, GeoAura)
	
	ATTRIBUTE_ACCESSORS(UAetherBaseAttributeSet, IncomingDamage)

public:
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats")
	FGameplayAttributeData Level;
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats")
	FGameplayAttributeData Health;
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats")
	FGameplayAttributeData MaxHealth;
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats")
	FGameplayAttributeData Atk;


	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData PhysicalDamageBonus;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData PyroDamageBonus;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData HydroDamageBonus;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData ElectroDamageBonus;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData AnemoDamageBonus;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData CryoDamageBonus;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData GeoDamageBonus;
	
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData PhysicalRes;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData PyroRes;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData HydroRes;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData ElectroRes;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData AnemoRes;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData CryoRes;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats")
	FGameplayAttributeData GeoRes;
	
	
	UPROPERTY(BlueprintReadOnly, Category = "Aura")
	FGameplayAttributeData PyroAura;
	UPROPERTY(BlueprintReadOnly, Category = "Aura")
	FGameplayAttributeData HydroAura;
	UPROPERTY(BlueprintReadOnly, Category = "Aura")
	FGameplayAttributeData ElectroAura;
	UPROPERTY(BlueprintReadOnly, Category = "Aura")
	FGameplayAttributeData CryoAura;
	UPROPERTY(BlueprintReadOnly, Category = "Aura")
	FGameplayAttributeData AnemoAura;
	UPROPERTY(BlueprintReadOnly, Category = "Aura")
	FGameplayAttributeData GeoAura;


	UPROPERTY(BlueprintReadOnly, Category= "Meta")
	FGameplayAttributeData IncomingDamage;
};
