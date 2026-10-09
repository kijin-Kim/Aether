// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherGameplayAbility_SwitchPartySlot.h"

#include "AbilitySystemComponent.h"
#include "Aether/AetherGameplayTags.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/Effects/AetherGameplayEffect_SwitchCooldown.h"
#include "Aether/PartySystem/AetherPartyComponent.h"
#include "Aether/Player/AetherPlayerState.h"


UAetherGameplayAbility_SwitchPartySlot::UAetherGameplayAbility_SwitchPartySlot()
{
	ActivationBlockedTags.AddTag(AetherGameplayTags::Cooldown_Ability_SwitchPartySlot);
	ActivationPolicy = EAetherAbilityActivationPolicy::OnInputTriggered;
	SwapPolicy = EAetherAbilitySwapPolicy::Cancel;
	CooldownGameplayEffectClass = UAetherGameplayEffect_SwitchCooldown::StaticClass();
}

void UAetherGameplayAbility_SwitchPartySlot::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                                           const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	const FGameplayTag InputTag = Spec.GetDynamicSpecSourceTags().GetByIndex(0);
	TargetSlotIndex = AetherGameplayTags::GetPartyIndexFromInputTag(InputTag);
}

void UAetherGameplayAbility_SwitchPartySlot::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                             const FGameplayAbilityActorInfo* ActorInfo,
                                                             const FGameplayAbilityActivationInfo ActivationInfo,
                                                             const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitCheck(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}


	bool bSwitched = false;
	if (AController* Controller = ActorInfo ? ActorInfo->PlayerController.Get() : nullptr)
	{
		if (AAetherPlayerState* PS = Controller->GetPlayerState<AAetherPlayerState>())
		{
			if (UAetherPartyComponent* PartyComponent = PS->GetPartyComponent())
			{
				bSwitched = PartyComponent->RequestSwitchPartySlot(TargetSlotIndex);
			}
		}
	}

	const bool bCommitted = bSwitched && CommitAbility(Handle, ActorInfo, ActivationInfo);
	EndAbility(Handle, ActorInfo, ActivationInfo, false, !bCommitted);
}
