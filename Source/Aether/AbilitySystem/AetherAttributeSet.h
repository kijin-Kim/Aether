// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "AetherAttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

class FLifetimeProperty;
/**
 * 
 */
UCLASS()
class AETHER_API UAetherAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAetherAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_Atk(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_Def(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_CritRate(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_CritDamage(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_EnergyRecharge(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_ElementalMastery(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_PyroDamageBonus(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_HydroDamageBonus(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_ElectroDamageBonus(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_AnemoDamageBonus(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_CryoDamageBonus(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_GeoDamageBonus(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_PyroAura(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_HydroAura(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_ElectroAura(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_AnemoAura(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_CryoAura(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_GeoAura(const FGameplayAttributeData& OldValue);


	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, Health)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, MaxHealth)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, Atk)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, Def)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, CritRate)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, CritDamage)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, EnergyRecharge)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, ElementalMastery)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, PyroDamageBonus)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, HydroDamageBonus)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, ElectroDamageBonus)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, AnemoDamageBonus)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, CryoDamageBonus)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, GeoDamageBonus)

	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, PyroAura)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, HydroAura)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, ElectroAura)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, AnemoAura)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, CryoAura)
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, GeoAura)
	
	ATTRIBUTE_ACCESSORS(UAetherAttributeSet, IncomingDamage)

public:
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "BaseStats", ReplicatedUsing = OnRep_Atk)
	FGameplayAttributeData Atk;
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats", ReplicatedUsing = OnRep_Def)
	FGameplayAttributeData Def;
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats", ReplicatedUsing = OnRep_CritRate)
	FGameplayAttributeData CritRate;
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats", ReplicatedUsing = OnRep_CritDamage)
	FGameplayAttributeData CritDamage;
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats", ReplicatedUsing = OnRep_EnergyRecharge)
	FGameplayAttributeData EnergyRecharge;
	UPROPERTY(BlueprintReadOnly, Category = "BaseStats", ReplicatedUsing = OnRep_ElementalMastery)
	FGameplayAttributeData ElementalMastery;

	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats", ReplicatedUsing = OnRep_PyroDamageBonus)
	FGameplayAttributeData PyroDamageBonus;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats", ReplicatedUsing = OnRep_HydroDamageBonus)
	FGameplayAttributeData HydroDamageBonus;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats", ReplicatedUsing = OnRep_ElectroDamageBonus)
	FGameplayAttributeData ElectroDamageBonus;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats", ReplicatedUsing = OnRep_AnemoDamageBonus)
	FGameplayAttributeData AnemoDamageBonus;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats", ReplicatedUsing = OnRep_CryoDamageBonus)
	FGameplayAttributeData CryoDamageBonus;
	UPROPERTY(BlueprintReadOnly, Category = "AdvancedStats", ReplicatedUsing = OnRep_GeoDamageBonus)
	FGameplayAttributeData GeoDamageBonus;


	UPROPERTY(BlueprintReadOnly, Category = "Elemental", ReplicatedUsing = OnRep_PyroAura)
	FGameplayAttributeData PyroAura;
	UPROPERTY(BlueprintReadOnly, Category = "Elemental", ReplicatedUsing = OnRep_HydroAura)
	FGameplayAttributeData HydroAura;
	UPROPERTY(BlueprintReadOnly, Category = "Elemental", ReplicatedUsing = OnRep_ElectroAura)
	FGameplayAttributeData ElectroAura;
	UPROPERTY(BlueprintReadOnly, Category = "Elemental", ReplicatedUsing = OnRep_CryoAura)
	FGameplayAttributeData CryoAura;
	UPROPERTY(BlueprintReadOnly, Category = "Elemental", ReplicatedUsing = OnRep_AnemoAura)
	FGameplayAttributeData AnemoAura;
	UPROPERTY(BlueprintReadOnly, Category = "Elemental", ReplicatedUsing = OnRep_GeoAura)
	FGameplayAttributeData GeoAura;


	UPROPERTY(BlueprintReadOnly, Category= "Meta")
	FGameplayAttributeData IncomingDamage;
};
