// Fill out your copyright notice in the Description page of Project Settings.

// STT_ActivateAbilityByTag.h
#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "STT_ActivateAbilityByTag.generated.h"

class AAetherAIController;

USTRUCT()
struct AETHER_API FSTT_ActivateAbilityByTagInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

USTRUCT(meta = (DisplayName = "Activate Ability By Tag", Category = "Aether|AI"))
struct AETHER_API FSTT_ActivateAbilityByTag : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_ActivateAbilityByTagInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};