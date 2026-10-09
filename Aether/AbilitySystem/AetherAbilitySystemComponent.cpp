// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherAbilitySystemComponent.h"

#include "AetherGameplayAbility.h"
#include "Abilities/AetherGameplayAbility_SwitchPartySlot.h"
#include "Aether/Aether.h"
#include "Aether/AetherGameplayTags.h"


UAetherAbilitySystemComponent::UAetherAbilitySystemComponent()
{
	SetIsReplicatedByDefault(false);
}

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
