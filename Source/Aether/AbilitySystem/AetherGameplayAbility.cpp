// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherGameplayAbility.h"

#include "AbilitySystemComponent.h"
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

void UAetherGameplayAbility::ApplyElementalAttackToTarget(AActor* Target, FGameplayTag ElementTypeTag, float Damage, float Gauge)
{
	if (!Target)
	{
		return;
	}
	
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(SourceASC->GetAvatarActor());
	Context.AddInstigator(SourceASC->GetAvatarActor(), SourceASC->GetAvatarActor());

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(UAetherGameplayEffect_ElementalDamage::StaticClass(), 1.0f, Context);
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	
	Spec->SetSetByCallerMagnitude(AetherGameplayTags::Data_Damage, Damage);
	Spec->SetSetByCallerMagnitude(AetherGameplayTags::Data_AuraGauge, Gauge);
	Spec->AddDynamicAssetTag(ElementTypeTag);

	FActiveGameplayEffectHandle AppliedHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*Spec, TargetASC);


}
