// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherGameplayEffect_ElementalDamage.h"

#include "Aether/AbilitySystem/ReactionAwareDamageExecution.h"

UAetherGameplayEffect_ElementalDamage::UAetherGameplayEffect_ElementalDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	FGameplayEffectExecutionDefinition ExecDef;
	ExecDef.CalculationClass = UReactionAwareDamageExecution::StaticClass();
	Executions.Add(ExecDef);
}
