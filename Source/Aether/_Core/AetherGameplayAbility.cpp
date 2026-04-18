// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherGameplayAbility.h"

#include "AbilitySystemComponent.h"

UAetherGameplayAbility::UAetherGameplayAbility()
	: ActivationPolicy(EAetherAbilityActivationPolicy::OnInputTriggered)

{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAetherGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (ActivationPolicy == EAetherAbilityActivationPolicy::OnGranted)
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
	}
}
