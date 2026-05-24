// Fill out your copyright notice in the Description page of Project Settings.


#include "ReactionAwareDamageExecution.h"

#include "AbilitySystemComponent.h"
#include "AetherElementalReactionResolver.h"
#include "AttributeSet/AetherBaseAttributeSet.h"
#include "Aether/AetherGameplayTags.h"
#include "AttributeSet/AetherHeroAttributeSet.h"


struct FReactionDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition AttackerLevelDef;
	FGameplayEffectAttributeCaptureDefinition DefenderLevelDef;
	DECLARE_ATTRIBUTE_CAPTUREDEF(Atk);

	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalDamageBonus);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PyroDamageBonus);
	DECLARE_ATTRIBUTE_CAPTUREDEF(HydroDamageBonus);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ElectroDamageBonus);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CryoDamageBonus);
	DECLARE_ATTRIBUTE_CAPTUREDEF(AnemoDamageBonus);
	DECLARE_ATTRIBUTE_CAPTUREDEF(GeoDamageBonus);

	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalRes);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PyroRes);
	DECLARE_ATTRIBUTE_CAPTUREDEF(HydroRes);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ElectroRes);
	DECLARE_ATTRIBUTE_CAPTUREDEF(AnemoRes);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CryoRes);
	DECLARE_ATTRIBUTE_CAPTUREDEF(GeoRes);

	DECLARE_ATTRIBUTE_CAPTUREDEF(PyroAura);
	DECLARE_ATTRIBUTE_CAPTUREDEF(HydroAura);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ElectroAura);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CryoAura);
	DECLARE_ATTRIBUTE_CAPTUREDEF(AnemoAura);
	DECLARE_ATTRIBUTE_CAPTUREDEF(GeoAura);


	DECLARE_ATTRIBUTE_CAPTUREDEF(CritRate);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(EnergyRecharge);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ElementalMastery);


	FReactionDamageStatics()
	{
		AttackerLevelDef = FGameplayEffectAttributeCaptureDefinition(
			UAetherBaseAttributeSet::GetLevelAttribute(),
			EGameplayEffectAttributeCaptureSource::Source,
			false
		);


		DefenderLevelDef = FGameplayEffectAttributeCaptureDefinition(
			UAetherBaseAttributeSet::GetLevelAttribute(),
			EGameplayEffectAttributeCaptureSource::Target,
			false
		);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, Atk, Source, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, PhysicalDamageBonus, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, PyroDamageBonus, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, HydroDamageBonus, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, ElectroDamageBonus, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, AnemoDamageBonus, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, CryoDamageBonus, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, GeoDamageBonus, Source, false);


		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, PhysicalRes, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, PyroRes, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, HydroRes, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, ElectroRes, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, AnemoRes, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, CryoRes, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, GeoRes, Target, false);


		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, PyroAura, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, HydroAura, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, ElectroAura, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, CryoAura, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, AnemoAura, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherBaseAttributeSet, GeoAura, Target, false);


		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherHeroAttributeSet, CritRate, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherHeroAttributeSet, CritDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherHeroAttributeSet, EnergyRecharge, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherHeroAttributeSet, ElementalMastery, Source, false);
	}
};

static const FReactionDamageStatics& Statics()
{
	static FReactionDamageStatics S;
	return S;
}

struct FDamageBonusSnapshot
{
	float Physical = 0.0f;
	float Pyro = 0.0f;
	float Hydro = 0.0f;
	float Electro = 0.0f;
	float Cryo = 0.0f;
	float Anemo = 0.0f;
	float Geo = 0.0f;
};

struct FResSnapShot
{
	float Physical = 0.0f;
	float Pyro = 0.0f;
	float Hydro = 0.0f;
	float Electro = 0.0f;
	float Cryo = 0.0f;
	float Anemo = 0.0f;
	float Geo = 0.0f;
};

static FGameplayTag GetElementTagFromSpec(const FGameplayEffectSpec& Spec)
{
	const FGameplayTagContainer& AssetTags = Spec.GetDynamicAssetTags();
	if (AssetTags.HasTagExact(AetherGameplayTags::Element_Pyro))
	{
		return AetherGameplayTags::Element_Pyro;
	}
	if (AssetTags.HasTagExact(AetherGameplayTags::Element_Hydro))
	{
		return AetherGameplayTags::Element_Hydro;
	}
	if (AssetTags.HasTagExact(AetherGameplayTags::Element_Electro))
	{
		return AetherGameplayTags::Element_Electro;
	}
	if (AssetTags.HasTagExact(AetherGameplayTags::Element_Cryo))
	{
		return AetherGameplayTags::Element_Cryo;
	}
	if (AssetTags.HasTagExact(AetherGameplayTags::Element_Anemo))
	{
		return AetherGameplayTags::Element_Anemo;
	}
	if (AssetTags.HasTagExact(AetherGameplayTags::Element_Geo))
	{
		return AetherGameplayTags::Element_Geo;
	}

	return FGameplayTag();
}

static float GetDamageBonusForElement(FGameplayTag ElementTag, const FDamageBonusSnapshot& DamageBonusSnapShot)
{
	if (ElementTag == AetherGameplayTags::Element_Pyro)
	{
		return DamageBonusSnapShot.Pyro;
	}
	if (ElementTag == AetherGameplayTags::Element_Hydro)
	{
		return DamageBonusSnapShot.Hydro;
	}
	if (ElementTag == AetherGameplayTags::Element_Electro)
	{
		return DamageBonusSnapShot.Electro;
	}
	if (ElementTag == AetherGameplayTags::Element_Cryo)
	{
		return DamageBonusSnapShot.Cryo;
	}
	if (ElementTag == AetherGameplayTags::Element_Anemo)
	{
		return DamageBonusSnapShot.Anemo;
	}
	if (ElementTag == AetherGameplayTags::Element_Geo)
	{
		return DamageBonusSnapShot.Geo;
	}

	return DamageBonusSnapShot.Physical;
}

static float GetResistanceForElement(FGameplayTag ElementTag, const FResSnapShot& ResSnapShot)
{
	if (ElementTag == AetherGameplayTags::Element_Pyro)
	{
		return ResSnapShot.Pyro;
	}
	if (ElementTag == AetherGameplayTags::Element_Hydro)
	{
		return ResSnapShot.Hydro;
	}
	if (ElementTag == AetherGameplayTags::Element_Electro)
	{
		return ResSnapShot.Electro;
	}
	if (ElementTag == AetherGameplayTags::Element_Cryo)
	{
		return ResSnapShot.Cryo;
	}
	if (ElementTag == AetherGameplayTags::Element_Anemo)
	{
		return ResSnapShot.Anemo;
	}
	if (ElementTag == AetherGameplayTags::Element_Geo)
	{
		return ResSnapShot.Geo;
	}

	return ResSnapShot.Physical;
}

static float CalculateResistanceMultiplier(float Resistance)
{
	if (Resistance < 0.0f)
	{
		return 1.0f - Resistance * 0.5f;
	}
	if (Resistance < 0.75f)
	{
		return 1.0f - Resistance;
	}

	return 1.0f / (4.0f * Resistance + 1.0f);
}


UReactionAwareDamageExecution::UReactionAwareDamageExecution()
{
	RelevantAttributesToCapture.Add(Statics().AttackerLevelDef);
	RelevantAttributesToCapture.Add(Statics().DefenderLevelDef);

	RelevantAttributesToCapture.Add(Statics().PhysicalDamageBonusDef);
	RelevantAttributesToCapture.Add(Statics().PyroDamageBonusDef);
	RelevantAttributesToCapture.Add(Statics().HydroDamageBonusDef);
	RelevantAttributesToCapture.Add(Statics().ElectroDamageBonusDef);
	RelevantAttributesToCapture.Add(Statics().CryoDamageBonusDef);
	RelevantAttributesToCapture.Add(Statics().AnemoDamageBonusDef);
	RelevantAttributesToCapture.Add(Statics().GeoDamageBonusDef);

	RelevantAttributesToCapture.Add(Statics().PhysicalResDef);
	RelevantAttributesToCapture.Add(Statics().PyroResDef);
	RelevantAttributesToCapture.Add(Statics().HydroResDef);
	RelevantAttributesToCapture.Add(Statics().ElectroResDef);
	RelevantAttributesToCapture.Add(Statics().AnemoResDef);
	RelevantAttributesToCapture.Add(Statics().CryoResDef);
	RelevantAttributesToCapture.Add(Statics().GeoResDef);


	RelevantAttributesToCapture.Add(Statics().PyroAuraDef);
	RelevantAttributesToCapture.Add(Statics().HydroAuraDef);
	RelevantAttributesToCapture.Add(Statics().ElectroAuraDef);
	RelevantAttributesToCapture.Add(Statics().CryoAuraDef);
	RelevantAttributesToCapture.Add(Statics().AnemoAuraDef);
	RelevantAttributesToCapture.Add(Statics().GeoAuraDef);


	RelevantAttributesToCapture.Add(Statics().AtkDef);

	RelevantAttributesToCapture.Add(Statics().CritRateDef);
	RelevantAttributesToCapture.Add(Statics().CritDamageDef);
	RelevantAttributesToCapture.Add(Statics().EnergyRechargeDef);
	RelevantAttributesToCapture.Add(Statics().ElementalMasteryDef);
}

void UReactionAwareDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecParams, FGameplayEffectCustomExecutionOutput& OutExecOutput) const
{
	Super::Execute_Implementation(ExecParams, OutExecOutput);
	const FGameplayEffectSpec& Spec = ExecParams.GetOwningSpec();

	UAbilitySystemComponent* SourceASC = ExecParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecParams.GetTargetAbilitySystemComponent();

	AActor* SourceActor = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetActor = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Atk = 0.f;
	float EM = 0.0f;
	float AttackerLevel = 0.0f;
	float DefenderLevel = 0.0f;

	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().AtkDef, EvalParams, Atk);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().ElementalMasteryDef, EvalParams, EM);


	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().AttackerLevelDef, EvalParams, AttackerLevel);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().DefenderLevelDef, EvalParams, DefenderLevel);


	FDamageBonusSnapshot DamageBonusSnapShot;

	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().PhysicalDamageBonusDef, EvalParams, DamageBonusSnapShot.Physical);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().PyroDamageBonusDef, EvalParams, DamageBonusSnapShot.Pyro);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().HydroDamageBonusDef, EvalParams, DamageBonusSnapShot.Hydro);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().ElectroDamageBonusDef, EvalParams, DamageBonusSnapShot.Electro);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().CryoDamageBonusDef, EvalParams, DamageBonusSnapShot.Cryo);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().AnemoDamageBonusDef, EvalParams, DamageBonusSnapShot.Anemo);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().GeoDamageBonusDef, EvalParams, DamageBonusSnapShot.Geo);


	FResSnapShot ResSnapShot;

	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().PhysicalResDef, EvalParams, ResSnapShot.Physical);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().PyroResDef, EvalParams, ResSnapShot.Pyro);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().HydroResDef, EvalParams, ResSnapShot.Hydro);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().ElectroResDef, EvalParams, ResSnapShot.Electro);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().CryoResDef, EvalParams, ResSnapShot.Cryo);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().AnemoResDef, EvalParams, ResSnapShot.Anemo);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().GeoResDef, EvalParams, ResSnapShot.Geo);

	FAetherAuraMagnitudeSnapshot AuraSnapShot;


	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().PyroAuraDef, EvalParams, AuraSnapShot.Pyro);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().HydroAuraDef, EvalParams, AuraSnapShot.Hydro);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().ElectroAuraDef, EvalParams, AuraSnapShot.Electro);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().CryoAuraDef, EvalParams, AuraSnapShot.Cryo);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().AnemoAuraDef, EvalParams, AuraSnapShot.Anemo);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().GeoAuraDef, EvalParams, AuraSnapShot.Geo);


	float BaseDamage = Spec.GetSetByCallerMagnitude(AetherGameplayTags::Data_Damage, false, 0.f);
	float AuraGauge = Spec.GetSetByCallerMagnitude(AetherGameplayTags::Data_AuraGauge, false, 0.f);
	const FGameplayTag IncomingElement = GetElementTagFromSpec(Spec);

	float CritRate = 0.0f;
	float CritDamage = 0.0f;
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().CritRateDef, EvalParams, CritRate);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().CritDamageDef, EvalParams, CritDamage);


	float AbilityDamage = 0.7008f;
	BaseDamage = Atk * AbilityDamage;

	const FAetherElementalReactionResult ReactionResult = FAetherElementalReactionResolver::Resolve(
		IncomingElement,
		AuraGauge,
		EM,
		AttackerLevel,
		AuraSnapShot);

	float DamageBonus = GetDamageBonusForElement(IncomingElement, DamageBonusSnapShot);
	float CritMultiplier = FMath::FRand() <= CritRate ? (1.0f + CritDamage) : 1.0f;
	float ReactionMultiplier = ReactionResult.DamageMultiplier;
	float DefenseMultiplier = (AttackerLevel + 100) / ((DefenderLevel + 100) + (AttackerLevel + 100));
	float ElementalResistance = GetResistanceForElement(IncomingElement, ResSnapShot);
	float ResistanceMultiplier = CalculateResistanceMultiplier(ElementalResistance);
	float AdditionalDamage = ReactionResult.AdditionalDamage;


	int FinalDamage = 0;
	FinalDamage = FMath::RoundToInt(
			BaseDamage
			* (1 + DamageBonus)
			* CritMultiplier
			* ReactionMultiplier
			* DefenseMultiplier
			* ResistanceMultiplier
			+ AdditionalDamage);



	if (FinalDamage > 0)
	{
		OutExecOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UAetherBaseAttributeSet::GetIncomingDamageAttribute(),
			EGameplayModOp::Additive, FinalDamage));
	}

	for (const FAetherAuraModifier& AuraModifier : ReactionResult.AuraModifiers)
	{
		if (AuraModifier.Attribute.IsValid() && !FMath::IsNearlyZero(AuraModifier.Magnitude))
		{
			OutExecOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
				AuraModifier.Attribute, EGameplayModOp::Additive, AuraModifier.Magnitude));
		}
	}
}
