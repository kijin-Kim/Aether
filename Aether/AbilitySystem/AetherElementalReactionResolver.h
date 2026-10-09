// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"

struct FAetherAuraMagnitudeSnapshot
{
	float Pyro = 0.0f;
	float Hydro = 0.0f;
	float Electro = 0.0f;
	float Cryo = 0.0f;
	float Anemo = 0.0f;
	float Geo = 0.0f;
};

struct FAetherAuraModifier
{
	FGameplayAttribute Attribute;
	float Magnitude = 0.0f;
};

struct FAetherElementalReactionResult
{
	FGameplayTag ReactionTag;
	float DamageMultiplier = 1.0f;
	float AdditionalDamage = 0.0f;
	TArray<FAetherAuraModifier> AuraModifiers;

	bool HasReaction() const { return ReactionTag.IsValid(); }
};

class AETHER_API FAetherElementalReactionResolver
{
public:
	static FAetherElementalReactionResult Resolve(
		FGameplayTag IncomingElement,
		float IncomingAuraGauge,
		float ElementalMastery,
		float SourceLevel,
		const FAetherAuraMagnitudeSnapshot& TargetAuras);

	static FGameplayAttribute GetAuraAttributeForElement(FGameplayTag ElementTag);
	static float GetAuraMagnitudeForElement(FGameplayTag ElementTag, const FAetherAuraMagnitudeSnapshot& TargetAuras);

private:
	static void AddAuraModifier(FAetherElementalReactionResult& Result, FGameplayTag ElementTag, float Magnitude);
	static float CalculateAmplifyingMultiplier(float BaseMultiplier, float ElementalMastery);
	static float CalculateTransformativeDamage(float SourceLevel, float ElementalMastery, float ReactionScale);
};
