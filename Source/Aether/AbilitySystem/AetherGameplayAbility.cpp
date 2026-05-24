// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Aether/Aether.h"
#include "Aether/AetherGameplayTags.h"
#include "Effects/AetherGameplayEffect_ElementalDamage.h"

UAetherGameplayAbility::UAetherGameplayAbility()
	: ActivationPolicy(EAetherAbilityActivationPolicy::OnInputTriggered)

{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAetherGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (ActivationPolicy == EAetherAbilityActivationPolicy::OnGranted)
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
	}
}

void UAetherGameplayAbility::ApplyElementalAttackToTarget(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ElementTypeTag, float Damage, float Gauge)
{

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		UE_LOG(LogAether, Warning, TEXT("ApplyElementalAttackToTarget failed: SourceASC is null."));
		return;
	}

	bool bHasValidTargetData = false;
	for (int32 Index = 0; Index < TargetDataHandle.Num(); ++Index)
	{
		if (TargetDataHandle.IsValid(Index))
		{
			bHasValidTargetData = true;
			break;
		}
	}

	if (!bHasValidTargetData)
	{
		UE_LOG(LogAether, Verbose, TEXT("ApplyElementalAttackToTarget skipped: target data is empty or invalid."));
		return;
	}

	if (!ElementTypeTag.IsValid())
	{
		UE_LOG(LogAether, Warning, TEXT("ApplyElementalAttackToTarget failed: ElementTypeTag is invalid."));
		return;
	}

	if (Damage <= 0.0f && Gauge <= 0.0f)
	{
		UE_LOG(LogAether, Verbose, TEXT("ApplyElementalAttackToTarget skipped: both damage and aura gauge are non-positive."));
		return;
	}

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(SourceASC->GetAvatarActor());
	Context.AddInstigator(SourceASC->GetAvatarActor(), SourceASC->GetAvatarActor());

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(UAetherGameplayEffect_ElementalDamage::StaticClass(), 1.0f, Context);
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogAether, Warning, TEXT("ApplyElementalAttackToTarget failed: could not create elemental damage spec."));
		return;
	}

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	if (!Spec)
	{
		UE_LOG(LogAether, Warning, TEXT("ApplyElementalAttackToTarget failed: elemental damage spec data is null."));
		return;
	}

	Spec->SetSetByCallerMagnitude(AetherGameplayTags::Data_Damage, Damage);
	Spec->SetSetByCallerMagnitude(AetherGameplayTags::Data_AuraGauge, Gauge);
	Spec->AddDynamicAssetTag(ElementTypeTag);
	TArray<FActiveGameplayEffectHandle> SpecHandles = ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), SpecHandle, TargetDataHandle);
}
