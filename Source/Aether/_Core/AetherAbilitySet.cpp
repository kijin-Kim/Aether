// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherAbilitySet.h"

#include "AetherAbilitySystemComponent.h"
#include "AetherGameplayAbility.h"
#include "GameplayAbilitySpec.h"

FAetherAbilitySet_GameplayAbility::FAetherAbilitySet_GameplayAbility()
	: Level(1)
{
}

void UAetherAbilitySet::InitializeAbilitySystem(UAetherAbilitySystemComponent* AetherASC, UObject* SourceObject) const
{
	for (const FAetherAbilitySet_GameplayAbility& Ability : Abilities)
	{
		FGameplayAbilitySpec AbilitySpec(Ability.Ability, Ability.Level, INDEX_NONE, SourceObject);
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(Ability.InputTag);
		AetherASC->GiveAbility(AbilitySpec);
	}
}
