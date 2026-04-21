// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Aether/AbilitySystem/AetherGameplayAbility.h"
#include "AetherGameplayAbility_SwitchPartySlot.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class AETHER_API UAetherGameplayAbility_SwitchPartySlotBase : public UAetherGameplayAbility
{
	GENERATED_BODY()
	
public:
	UAetherGameplayAbility_SwitchPartySlotBase();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Aether|SwitchPartySlot")
	int32 TargetSlotIndex;
};