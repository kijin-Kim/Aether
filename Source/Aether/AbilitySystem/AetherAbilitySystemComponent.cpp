// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherAbilitySystemComponent.h"

#include "AetherGameplayAbility.h"
#include "Abilities/AetherGameplayAbility_SwitchPartySlot.h"
#include "Aether/Aether.h"
#include "Aether/AetherGameplayTags.h"


void UAetherAbilitySystemComponent::ProcessInputs()
{
	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (AbilitySpec && AbilitySpec->Ability)
		{
			AbilitySpec->InputPressed = true;
			if (!AbilitySpec->IsActive())
			{
				const UAetherGameplayAbility* Ability = Cast<UAetherGameplayAbility>(AbilitySpec->Ability);
				if (Ability && Ability->GetActivationPolicy() == EAetherAbilityActivationPolicy::OnInputTriggered)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
			else
			{
				AbilitySpecInputPressed(*AbilitySpec);
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (AbilitySpec && AbilitySpec->Ability && !AbilitySpec->IsActive())
		{
			const UAetherGameplayAbility* Ability = Cast<UAetherGameplayAbility>(AbilitySpec->Ability);
			if (Ability && Ability->GetActivationPolicy() == EAetherAbilityActivationPolicy::OnInputHeld)
			{
				AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		bool bResult = TryActivateAbility(AbilitySpecHandle);
		if (bResult)
		{
			UE_LOG(LogAether, Log, TEXT("ProcessInputs: Ability Activated: %s"), *GetNameSafe(FindAbilitySpecFromHandle(AbilitySpecHandle)->Ability));
		}
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (AbilitySpec && AbilitySpec->Ability)
		{
			AbilitySpec->InputPressed = false;
			if (AbilitySpec->IsActive())
			{
				AbilitySpecInputReleased(*AbilitySpec);
			}
		}
	}

	ClearAbilityInputs();
}

void UAetherAbilitySystemComponent::ClearAbilityInputs()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UAetherAbilitySystemComponent::ClearAllAbilityInputs()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

bool UAetherAbilitySystemComponent::HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy SwapPolicy) const
{
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		const UAetherGameplayAbility* AetherAbility = Cast<UAetherGameplayAbility>(AbilitySpec.Ability);
		if (AetherAbility && AetherAbility->GetSwapPolicy() == SwapPolicy)
		{
			return true;
		}
	}

	return false;
}

void UAetherAbilitySystemComponent::CancelActiveAbilitiesWithSwapPolicy(EAetherAbilitySwapPolicy SwapPolicy)
{
	TArray<FGameplayAbilitySpecHandle> HandlesToCancel;
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		const UAetherGameplayAbility* AetherAbility = Cast<UAetherGameplayAbility>(AbilitySpec.Ability);
		if (AetherAbility && AetherAbility->GetSwapPolicy() == SwapPolicy)
		{
			HandlesToCancel.Add(AbilitySpec.Handle);
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : HandlesToCancel)
	{
		CancelAbilityHandle(Handle);
	}
}

void UAetherAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	ClearAllAbilityInputs();
}

void UAetherAbilitySystemComponent::AbilityInputPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UAetherAbilitySystemComponent::AbilityInputReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void UAetherAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	if (Spec.Ability && Spec.IsActive())
	{
		check(Spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerActor);
		if (const UGameplayAbility* Instance = Spec.GetPrimaryInstance())
		{
			FPredictionKey PredictionKey = Instance->GetCurrentActivationInfo().GetActivationPredictionKey();
			// InputPress/Release Ability Task를 위한 이벤트를 호출. 
			// InstancedPerActor에서만 동작하도록 지원.
			// NonInstanced는 Deprecated
			// InstancedPerExecution은 입력 이벤트가 왔을 때, 어떤 인스턴스에 이벤트를 전달해야 하는지 논리적으로 애매함
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, PredictionKey);
		}
	}
}

void UAetherAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	if (Spec.IsActive())
	{
		check(Spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerActor);
		if (const UGameplayAbility* Instance = Spec.GetPrimaryInstance())
		{
			FPredictionKey PredictionKey = Instance->GetCurrentActivationInfo().GetActivationPredictionKey();
			// InputPress/Release Ability Task를 위한 이벤트를 호출. 
			// InstancedPerActor에서만 동작하도록 지원.
			// NonInstanced는 Deprecated
			// InstancedPerExecution은 입력 이벤트가 왔을 때, 어떤 인스턴스에 이벤트를 전달해야 하는지 논리적으로 애매함
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, PredictionKey);
		}
	}
}
