// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherGameplayEffect_SwitchCooldown.h"

#include "Aether/AetherGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UAetherGameplayEffect_SwitchCooldown::UAetherGameplayEffect_SwitchCooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(1.0f);

	UTargetTagsGameplayEffectComponent* TargetTagsComponent = CreateDefaultSubobject<
		UTargetTagsGameplayEffectComponent>(TEXT("CooldownTagsComponent"));

	GEComponents.Add(TargetTagsComponent);

	FInheritedTagContainer CooldownTags;
	CooldownTags.AddTag(AetherGameplayTags::Cooldown_Ability_SwitchPartySlot);

	TargetTagsComponent->SetAndApplyTargetTagChanges(CooldownTags);
}
