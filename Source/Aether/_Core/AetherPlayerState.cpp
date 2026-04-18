// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPlayerState.h"

#include "AetherAbilitySystemComponent.h"
#include "AetherAttributeSet.h"

AAetherPlayerState::AAetherPlayerState()
{
	AetherASC = CreateDefaultSubobject<UAetherAbilitySystemComponent>("AbilitySystemComponent");
	AetherASC->SetIsReplicated(true);
	AetherASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<UAetherAttributeSet>("AttributeSet");
	
	SetNetUpdateFrequency(30.0f);
}

UAbilitySystemComponent* AAetherPlayerState::GetAbilitySystemComponent() const
{
	 return AetherASC;
}
