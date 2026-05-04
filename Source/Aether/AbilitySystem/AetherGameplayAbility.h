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
	OnInputRepeat,
	OnGranted
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
	void ApplyElementalAttackToTarget(AActor* Target, FGameplayTag ElementTypeTag, float Damage, float Gauge);

	EAetherAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	

private:
	UPROPERTY(EditDefaultsOnly, Category = "Aether|Ability Activation")
	EAetherAbilityActivationPolicy ActivationPolicy;
};
