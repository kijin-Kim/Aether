// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherGameplayAbility_NormalAttack.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Aether/AetherGameplayTags.h"

void UAetherGameplayAbility_NormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	PlayCombo();
}

void UAetherGameplayAbility_NormalAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	ComboIndex = 0;
	bComboWindowOpened = false;
	bInputBuffered = false;
}

void UAetherGameplayAbility_NormalAttack::PlayCombo()
{
	check(Animations.IsValidIndex(ComboIndex));

	bComboWindowOpened = false;
	bInputBuffered = false;

	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Animations[ComboIndex]);
	PlayMontageTask->OnCompleted.AddDynamic(this, &UAetherGameplayAbility_NormalAttack::OnMontageCompleted);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UAetherGameplayAbility_NormalAttack::OnMontageCancelled);
	PlayMontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WindowOpenedTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AetherGameplayTags::NormalAttack_ComboWindow_Opened);
	WindowOpenedTask->EventReceived.AddDynamic(this, &UAetherGameplayAbility_NormalAttack::OnWindowOpenedEvent);
	WindowOpenedTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WindowClosedTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AetherGameplayTags::NormalAttack_ComboWindow_Closed);
	WindowClosedTask->EventReceived.AddDynamic(this, &UAetherGameplayAbility_NormalAttack::OnWindowClosedEvent);
	WindowClosedTask->ReadyForActivation();

	UAbilityTask_WaitInputPress* WaitInputTask = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputTask->OnPress.AddDynamic(this, &UAetherGameplayAbility_NormalAttack::OnInputPressedEvent);
	WaitInputTask->ReadyForActivation();

	ComboIndex = (ComboIndex + 1) % Animations.Num();
}

void UAetherGameplayAbility_NormalAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAetherGameplayAbility_NormalAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAetherGameplayAbility_NormalAttack::OnWindowOpenedEvent(FGameplayEventData Payload)
{
	bComboWindowOpened = true;
	if (bInputBuffered)
	{
		PlayCombo();
	}
}

void UAetherGameplayAbility_NormalAttack::OnWindowClosedEvent(FGameplayEventData Payload)
{
	bComboWindowOpened = false;
}

void UAetherGameplayAbility_NormalAttack::OnInputPressedEvent(float TimeWaited)
{
	if (bComboWindowOpened)
	{
		PlayCombo();
	}
	else
	{
		bInputBuffered = true;
	}
}
