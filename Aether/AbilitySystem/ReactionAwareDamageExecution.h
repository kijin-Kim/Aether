// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ReactionAwareDamageExecution.generated.h"




/**
 * 
 */
UCLASS()
class AETHER_API UReactionAwareDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UReactionAwareDamageExecution();
	
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecParams, FGameplayEffectCustomExecutionOutput& OutExecOutput) const override;
};