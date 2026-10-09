// Fill out your copyright notice in the Description page of Project Settings.

#include "AetherElementalReactionResolver.h"

#include "Aether/AetherGameplayTags.h"
#include "AttributeSet/AetherBaseAttributeSet.h"

FAetherElementalReactionResult FAetherElementalReactionResolver::Resolve(
	FGameplayTag IncomingElement,
	float IncomingAuraGauge,
	float ElementalMastery,
	float SourceLevel,
	const FAetherAuraMagnitudeSnapshot& TargetAuras)
{
	FAetherElementalReactionResult Result;

	if (!IncomingElement.IsValid() || IncomingAuraGauge <= 0.0f)
	{
		return Result;
	}

	const auto TryAmplifyingReaction = [&Result, IncomingAuraGauge, ElementalMastery](
		FGameplayTag ExistingElement,
		float ExistingAura,
		FGameplayTag ReactionTag,
		float Multiplier)
	{
		if (ExistingAura <= 0.0f)
		{
			return false;
		}

		Result.ReactionTag = ReactionTag;
		Result.DamageMultiplier = CalculateAmplifyingMultiplier(Multiplier, ElementalMastery);
		AddAuraModifier(Result, ExistingElement, -FMath::Min(ExistingAura, IncomingAuraGauge));
		return true;
	};

	const auto TryTransformativeReaction = [&Result, IncomingAuraGauge, ElementalMastery, SourceLevel](
		FGameplayTag ExistingElement,
		float ExistingAura,
		FGameplayTag ReactionTag,
		float ReactionScale)
	{
		if (ExistingAura <= 0.0f)
		{
			return false;
		}

		Result.ReactionTag = ReactionTag;
		Result.AdditionalDamage = CalculateTransformativeDamage(SourceLevel, ElementalMastery, ReactionScale);
		AddAuraModifier(Result, ExistingElement, -FMath::Min(ExistingAura, IncomingAuraGauge));
		return true;
	};

	if (IncomingElement == AetherGameplayTags::Element_Pyro)
	{
		if (TryAmplifyingReaction(AetherGameplayTags::Element_Hydro, TargetAuras.Hydro, AetherGameplayTags::Reaction_Vaporize, 1.5f)
			|| TryAmplifyingReaction(AetherGameplayTags::Element_Cryo, TargetAuras.Cryo, AetherGameplayTags::Reaction_Melt, 2.0f)
			|| TryTransformativeReaction(AetherGameplayTags::Element_Electro, TargetAuras.Electro, AetherGameplayTags::Reaction_Overloaded, 2.0f))
		{
			return Result;
		}
	}
	else if (IncomingElement == AetherGameplayTags::Element_Hydro)
	{
		if (TryAmplifyingReaction(AetherGameplayTags::Element_Pyro, TargetAuras.Pyro, AetherGameplayTags::Reaction_Vaporize, 2.0f)
			|| TryTransformativeReaction(AetherGameplayTags::Element_Electro, TargetAuras.Electro, AetherGameplayTags::Reaction_ElectroCharged, 1.2f))
		{
			return Result;
		}
	}
	else if (IncomingElement == AetherGameplayTags::Element_Cryo)
	{
		if (TryAmplifyingReaction(AetherGameplayTags::Element_Pyro, TargetAuras.Pyro, AetherGameplayTags::Reaction_Melt, 1.5f)
			|| TryTransformativeReaction(AetherGameplayTags::Element_Electro, TargetAuras.Electro, AetherGameplayTags::Reaction_Superconduct, 0.5f))
		{
			return Result;
		}
	}
	else if (IncomingElement == AetherGameplayTags::Element_Electro)
	{
		if (TryTransformativeReaction(AetherGameplayTags::Element_Pyro, TargetAuras.Pyro, AetherGameplayTags::Reaction_Overloaded, 2.0f)
			|| TryTransformativeReaction(AetherGameplayTags::Element_Hydro, TargetAuras.Hydro, AetherGameplayTags::Reaction_ElectroCharged, 1.2f)
			|| TryTransformativeReaction(AetherGameplayTags::Element_Cryo, TargetAuras.Cryo, AetherGameplayTags::Reaction_Superconduct, 0.5f))
		{
			return Result;
		}
	}

	AddAuraModifier(Result, IncomingElement, IncomingAuraGauge);
	return Result;
}

FGameplayAttribute FAetherElementalReactionResolver::GetAuraAttributeForElement(FGameplayTag ElementTag)
{
	if (ElementTag == AetherGameplayTags::Element_Pyro)
	{
		return UAetherBaseAttributeSet::GetPyroAuraAttribute();
	}
	if (ElementTag == AetherGameplayTags::Element_Hydro)
	{
		return UAetherBaseAttributeSet::GetHydroAuraAttribute();
	}
	if (ElementTag == AetherGameplayTags::Element_Electro)
	{
		return UAetherBaseAttributeSet::GetElectroAuraAttribute();
	}
	if (ElementTag == AetherGameplayTags::Element_Cryo)
	{
		return UAetherBaseAttributeSet::GetCryoAuraAttribute();
	}
	if (ElementTag == AetherGameplayTags::Element_Anemo)
	{
		return UAetherBaseAttributeSet::GetAnemoAuraAttribute();
	}
	if (ElementTag == AetherGameplayTags::Element_Geo)
	{
		return UAetherBaseAttributeSet::GetGeoAuraAttribute();
	}

	return FGameplayAttribute();
}

float FAetherElementalReactionResolver::GetAuraMagnitudeForElement(
	FGameplayTag ElementTag,
	const FAetherAuraMagnitudeSnapshot& TargetAuras)
{
	if (ElementTag == AetherGameplayTags::Element_Pyro)
	{
		return TargetAuras.Pyro;
	}
	if (ElementTag == AetherGameplayTags::Element_Hydro)
	{
		return TargetAuras.Hydro;
	}
	if (ElementTag == AetherGameplayTags::Element_Electro)
	{
		return TargetAuras.Electro;
	}
	if (ElementTag == AetherGameplayTags::Element_Cryo)
	{
		return TargetAuras.Cryo;
	}
	if (ElementTag == AetherGameplayTags::Element_Anemo)
	{
		return TargetAuras.Anemo;
	}
	if (ElementTag == AetherGameplayTags::Element_Geo)
	{
		return TargetAuras.Geo;
	}

	return 0.0f;
}

void FAetherElementalReactionResolver::AddAuraModifier(
	FAetherElementalReactionResult& Result,
	FGameplayTag ElementTag,
	float Magnitude)
{
	if (FMath::IsNearlyZero(Magnitude))
	{
		return;
	}

	FGameplayAttribute Attribute = GetAuraAttributeForElement(ElementTag);
	if (!Attribute.IsValid())
	{
		return;
	}

	FAetherAuraModifier Modifier;
	Modifier.Attribute = Attribute;
	Modifier.Magnitude = Magnitude;
	Result.AuraModifiers.Add(Modifier);
}

float FAetherElementalReactionResolver::CalculateAmplifyingMultiplier(float BaseMultiplier, float ElementalMastery)
{
	const float EMBonus = (2.78f * ElementalMastery) / (ElementalMastery + 1400.0f);
	return BaseMultiplier * (1.0f + FMath::Max(0.0f, EMBonus));
}

float FAetherElementalReactionResolver::CalculateTransformativeDamage(
	float SourceLevel,
	float ElementalMastery,
	float ReactionScale)
{
	const float Level = FMath::Max(1.0f, SourceLevel);
	const float BaseDamage = 12.0f + Level * 3.0f;
	const float EMBonus = (16.0f * ElementalMastery) / (ElementalMastery + 2000.0f);
	return BaseDamage * ReactionScale * (1.0f + FMath::Max(0.0f, EMBonus));
}
