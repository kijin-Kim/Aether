// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherGameplayAbility_NormalAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Aether/Aether.h"
#include "Aether/AetherGameplayTags.h"

void UAetherGameplayAbility_NormalAttack::OnAttackHit(FGameplayEventData Payload)
{
	if (Payload.TargetData.Num() == 0)
	{
		UE_LOG(LogAether, Verbose, TEXT("NormalAttack hit event ignored: no target data. Target=%s"), *GetNameSafe(Payload.Target));
		return;
	}

	ApplyElementalAttackToTarget(Payload.TargetData, AetherGameplayTags::Element_Pyro, 1.0f, 1.0f);
	UE_LOG(LogAether, Verbose, TEXT("NormalAttack hit event received. Target=%s"), *GetNameSafe(Payload.Target));
}

void UAetherGameplayAbility_NormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!HasValidComboData())
	{
		UE_LOG(LogAether, Warning, TEXT("NormalAttack activation failed: no valid montage data."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	
	UAbilityTask_WaitGameplayEvent* AttackHit = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AetherGameplayTags::Event_Montage_AttackHit);
	AttackHit->EventReceived.AddDynamic(this, &UAetherGameplayAbility_NormalAttack::OnAttackHit);
	AttackHit->ReadyForActivation();

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
	if (!GroundAttackAnimations.IsValidIndex(ComboIndex) || !GroundAttackAnimations[ComboIndex])
	{
		UE_LOG(LogAether, Warning, TEXT("NormalAttack combo stopped: invalid montage at index %d."), ComboIndex);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	bComboWindowOpened = false;
	bInputBuffered = false;

	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, GroundAttackAnimations[ComboIndex]);
	if (!PlayMontageTask)
	{
		UE_LOG(LogAether, Warning, TEXT("NormalAttack combo stopped: failed to create montage task."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	PlayMontageTask->OnCompleted.AddDynamic(this, &UAetherGameplayAbility_NormalAttack::OnMontageCompleted);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UAetherGameplayAbility_NormalAttack::OnMontageCancelled);
	PlayMontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WindowOpenedTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AetherGameplayTags::Event_NormalAttack_ComboWindow_Opened);
	WindowOpenedTask->EventReceived.AddDynamic(this, &UAetherGameplayAbility_NormalAttack::OnWindowOpenedEvent);
	WindowOpenedTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WindowClosedTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AetherGameplayTags::Event_NormalAttack_ComboWindow_Closed);
	WindowClosedTask->EventReceived.AddDynamic(this, &UAetherGameplayAbility_NormalAttack::OnWindowClosedEvent);
	WindowClosedTask->ReadyForActivation();

	UAbilityTask_WaitInputPress* WaitInputTask = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputTask->OnPress.AddDynamic(this, &UAetherGameplayAbility_NormalAttack::OnInputPressedEvent);
	WaitInputTask->ReadyForActivation();

	ComboIndex = (ComboIndex + 1) % GroundAttackAnimations.Num();
}

bool UAetherGameplayAbility_NormalAttack::HasValidComboData() const
{
	if (GroundAttackAnimations.IsEmpty())
	{
		return false;
	}

	for (const TObjectPtr<UAnimMontage>& Montage : GroundAttackAnimations)
	{
		if (!Montage)
		{
			return false;
		}
	}

	return true;
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
