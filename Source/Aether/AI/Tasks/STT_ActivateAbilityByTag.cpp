// Fill out your copyright notice in the Description page of Project Settings.


// STT_ActivateAbilityByTag.cpp
#include "STT_ActivateAbilityByTag.h"

#include "Aether/AI/AetherAIController.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FSTT_ActivateAbilityByTag::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (AAetherAIController* AIController = Cast<AAetherAIController>(Context.GetOwner()))
	{
		const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		AIController->TryActivateAbilityByTag(InstanceData.InputTag);
		return EStateTreeRunStatus::Succeeded;
	}
	return EStateTreeRunStatus::Failed;
}