// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "AetherGameplayAbility.generated.h"

UENUM(BlueprintType)
enum class EAetherAbilityActivationPolicy : uint8
{
	OnInputTriggered,
	OnInputHeld,
	OnGranted
};

UENUM(BlueprintType)
enum class EAetherAbilitySwapPolicy : uint8
{
	Cancel,
	AllowOffField,
	Block
};

/**
 * 
 */
UCLASS()
class AETHER_API UAetherGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
	
public:
	UAetherGameplayAbility();
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	void ApplyElementalAttackToTarget(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ElementTypeTag, float Damage, float Gauge);

	EAetherAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	EAetherAbilitySwapPolicy GetSwapPolicy() const { return SwapPolicy; }
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Aether|Ability Activation")
	EAetherAbilityActivationPolicy ActivationPolicy;

	UPROPERTY(EditDefaultsOnly, Category = "Aether|Party")
	EAetherAbilitySwapPolicy SwapPolicy;
};
