// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherAbilitySystemComponent.h"

#include "AetherGameplayAbility.h"


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

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputRepeatSpecHandle)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (AbilitySpec && AbilitySpec->Ability && !AbilitySpec->IsActive())
		{
			const UAetherGameplayAbility* Ability = Cast<UAetherGameplayAbility>(AbilitySpec->Ability);
			if (Ability && Ability->GetActivationPolicy() == EAetherAbilityActivationPolicy::OnInputRepeat)
			{
				AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
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

	ClearInputs();
}

void UAetherAbilitySystemComponent::ClearInputs()
{
	InputPressedSpecHandles.Reset();
	InputRepeatSpecHandle.Reset();
	InputReleasedSpecHandles.Reset();
}

void UAetherAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	ClearInputs();
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
				InputRepeatSpecHandle.AddUnique(AbilitySpec.Handle);
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
				InputRepeatSpecHandle.Remove(AbilitySpec.Handle);
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
