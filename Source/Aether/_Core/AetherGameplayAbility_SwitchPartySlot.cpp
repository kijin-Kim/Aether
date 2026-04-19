// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherGameplayAbility_SwitchPartySlot.h"

#include "AetherGameplayTags.h"
#include "AetherPlayerState.h"


UAetherGameplayAbility_SwitchPartySlotBase::UAetherGameplayAbility_SwitchPartySlotBase()
{
	ActivationBlockedTags.AddTag(AetherGameplayTags::Cooldown_Ability_SwitchPartySlot);
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UAetherGameplayAbility_SwitchPartySlotBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	if (APawn* AvatarPawn = Cast<APawn>(ActorInfo->AvatarActor.Get()))
	{
		if (AAetherPlayerState* AetherASC = AvatarPawn->GetPlayerState<AAetherPlayerState>())
		{
			AetherASC->AuthSwitchPartySlot(TargetSlotIndex);
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
	}
}
