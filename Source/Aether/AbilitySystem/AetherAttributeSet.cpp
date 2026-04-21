// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"


UAetherAttributeSet::UAetherAttributeSet()
	: Health(100.0f)
	  , MaxHealth(100.0f)
{
}

void UAetherAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UAetherAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, Health, OldValue);
}

void UAetherAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, MaxHealth, OldValue);
}
