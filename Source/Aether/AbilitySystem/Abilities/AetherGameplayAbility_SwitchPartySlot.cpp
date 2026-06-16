// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherGameplayAbility_SwitchPartySlot.h"

#include "AbilitySystemComponent.h"
#include "Aether/AetherGameplayTags.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/Character/AetherCharacter.h"
#include "Aether/Player/AetherPartyComponent.h"
#include "Aether/Player/AetherPlayerController.h"


UAetherGameplayAbility_SwitchPartySlot::UAetherGameplayAbility_SwitchPartySlot()
{
	ActivationBlockedTags.AddTag(AetherGameplayTags::Cooldown_Ability_SwitchPartySlot);
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	SwapPolicy = EAetherAbilitySwapPolicy::AllowOffField;
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
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	bool bSwitched = false;
	if (AController* Controller = ActorInfo ? ActorInfo->PlayerController.Get() : nullptr)
	{
		if (AAetherPlayerController* AetherPlayerController = Cast<AAetherPlayerController>(Controller))
		{
			if (UAetherPartyComponent* PartyComponent = AetherPlayerController->GetPartyComponent())
			{
				bSwitched = PartyComponent->RequestSwitchPartySlot(TargetSlotIndex);
			}
		}
	}

	const bool bCommitted = bSwitched && CommitAbility(Handle, ActorInfo, ActivationInfo);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, !bCommitted);
}
