// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherAbilitySet.h"

#include "AetherAbilitySystemComponent.h"
#include "AetherGameplayAbility.h"
#include "GameplayAbilitySpec.h"

void FAetherAbilitySet_GrantedHandles::ClearAbilitySystem(UAetherAbilitySystemComponent* AetherASC)
{
	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			AetherASC->ClearAbility(Handle);
		}
	}

	AbilitySpecHandles.Reset();

	for (const FActiveGameplayEffectHandle& Handle : EffectSpecHandles)
	{
		if (Handle.IsValid())
		{
			AetherASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	EffectSpecHandles.Reset();

	for (UAttributeSet* Set : AttributeSets)
	{
		if (Set)
		{
			AetherASC->RemoveSpawnedAttribute(Set);
		}
	}

	AttributeSets.Reset();
}

FAetherAbilitySet_GameplayAbility::FAetherAbilitySet_GameplayAbility()
	: Level(1)
{
}

void UAetherAbilitySet::InitializeAbilitySystem(UAetherAbilitySystemComponent* AetherASC, FAetherAbilitySet_GrantedHandles& OutGrantedHandles, UObject* SourceObject) const
{
	for (const FAetherAbilitySet_AttributeSet& AttributeSet : AttributeSets)
	{
		if (AttributeSet.AttributeSet)
		{
			UAttributeSet* NewSet = NewObject<UAttributeSet>(AetherASC->GetOwner(), AttributeSet.AttributeSet);
			AetherASC->AddAttributeSetSubobject(NewSet);
			OutGrantedHandles.AttributeSets.AddUnique(NewSet);
		}
	}

	for (const FAetherAbilitySet_GameplayAbility& Ability : Abilities)
	{
		FGameplayAbilitySpec AbilitySpec(Ability.Ability, Ability.Level, INDEX_NONE, SourceObject);
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(Ability.InputTag);
		OutGrantedHandles.AbilitySpecHandles.AddUnique(AetherASC->GiveAbility(AbilitySpec));
	}

	for (const FAetherAbilitySet_GameplayEffect& Effect : Effects)
	{
		if (Effect.Effect)
		{
			const FActiveGameplayEffectHandle EffectHandle = AetherASC->ApplyGameplayEffectToSelf(Effect.Effect->GetDefaultObject<UGameplayEffect>(), Effect.Level, AetherASC->MakeEffectContext());
			OutGrantedHandles.EffectSpecHandles.AddUnique(EffectHandle);
		}
	}

}
