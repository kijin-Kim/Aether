// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AetherInputConfig.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "AetherInputComponent.generated.h"


class UAetherInputConfig;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AETHER_API UAetherInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	template <class UserClass, typename FuncType>
	void BindInputAction(const UAetherInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func)
	{
		check(InputConfig);
		for (const FAetherInputAction& Action : InputConfig->InputActions)
		{
			if (Action.InputAction && Action.InputTag.IsValid() && Action.InputTag == InputTag)
			{
				BindAction(Action.InputAction, TriggerEvent, Object, Func);
				return;
			}
		}
	}

	template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityInputAction(const UAetherInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc)
	{
		check(InputConfig);
		for (const FAetherInputAction& Action : InputConfig->AbilityInputActions)
		{
			if (Action.InputAction && Action.InputTag.IsValid())
			{
				if (PressedFunc)
				{
					BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag);
				}
				if (ReleasedFunc)
				{
					BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
				}
			}
		}
	}
};
