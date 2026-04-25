// Fill out your copyright notice in the Description page of Project Settings.


#include "ReactionAwareDamageExecution.h"

#include "AbilitySystemComponent.h"
#include "AetherAttributeSet.h"
#include "Aether/AetherGameplayTags.h"


struct FReactionDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Atk);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Def);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ElementalMastery);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PyroAura);
	DECLARE_ATTRIBUTE_CAPTUREDEF(HydroAura);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ElectroAura);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CryoAura);
	DECLARE_ATTRIBUTE_CAPTUREDEF(AnemoAura);
	DECLARE_ATTRIBUTE_CAPTUREDEF(GeoAura);

	FReactionDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherAttributeSet, Atk, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherAttributeSet, Def, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherAttributeSet, ElementalMastery, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherAttributeSet, PyroAura, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherAttributeSet, HydroAura, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherAttributeSet, ElectroAura, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherAttributeSet, CryoAura, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherAttributeSet, AnemoAura, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAetherAttributeSet, GeoAura, Target, false);
	}
};

static const FReactionDamageStatics& Statics()
{
	static FReactionDamageStatics S;
	return S;
}


UReactionAwareDamageExecution::UReactionAwareDamageExecution()
{
	RelevantAttributesToCapture.Add(Statics().AtkDef);
	RelevantAttributesToCapture.Add(Statics().DefDef);
	RelevantAttributesToCapture.Add(Statics().ElementalMasteryDef);
	RelevantAttributesToCapture.Add(Statics().PyroAuraDef);
	RelevantAttributesToCapture.Add(Statics().HydroAuraDef);
	RelevantAttributesToCapture.Add(Statics().ElectroAuraDef);
	RelevantAttributesToCapture.Add(Statics().CryoAuraDef);
	RelevantAttributesToCapture.Add(Statics().AnemoAuraDef);
	RelevantAttributesToCapture.Add(Statics().GeoAuraDef);
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

	// 1. 캡처 값 읽기
	float Attack = 0.f, Defense = 0.f, EM = 0.f;
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().AtkDef, EvalParams, Attack);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().DefDef, EvalParams, Defense);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().ElementalMasteryDef, EvalParams, EM);


	struct FAuraSnapshot
	{
		float Pyro = 0;
		float Hydro = 0;
		float Electro = 0;
		float Cryo = 0;
		float Anemo = 0;
		float Geo = 0;
	} Snapshot;


	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().PyroAuraDef, EvalParams, Snapshot.Pyro);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().HydroAuraDef, EvalParams, Snapshot.Hydro);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().ElectroAuraDef, EvalParams, Snapshot.Electro);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().CryoAuraDef, EvalParams, Snapshot.Cryo);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().AnemoAuraDef, EvalParams, Snapshot.Anemo);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(Statics().GeoAuraDef, EvalParams, Snapshot.Geo);

	float BaseDamage = Spec.GetSetByCallerMagnitude(AetherGameplayTags::Data_Damage, false, 0.f);
	float AuraGauge = Spec.GetSetByCallerMagnitude(AetherGameplayTags::Data_AuraGauge, false, 0.f);


	float FinalDamage = 100.0f;
	if (FinalDamage > 0.f)
	{
		OutExecOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UAetherAttributeSet::GetIncomingDamageAttribute(),
			EGameplayModOp::Additive, FinalDamage));
	}

	FGameplayAttribute AuraAttr = UAetherAttributeSet::GetPyroAuraAttribute();
	if (AuraAttr.IsValid())
	{
		OutExecOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			AuraAttr, EGameplayModOp::Additive, 1.0f));
	}
}
