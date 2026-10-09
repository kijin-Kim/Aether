// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Aether/AetherGameplayTags.h"
#include "Aether/AbilitySystem/AetherGameplayAbility.h"
#include "AetherGameplayAbility_SwitchPartySlot.generated.h"


/**
 * 
 */
UCLASS()
class AETHER_API UAetherGameplayAbility_SwitchPartySlot : public UAetherGameplayAbility
{
	GENERATED_BODY()

public:
	UAetherGameplayAbility_SwitchPartySlot();
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

private:
	int32 TargetSlotIndex = INDEX_NONE;
};
