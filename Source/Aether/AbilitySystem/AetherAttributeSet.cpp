// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"


UAetherAttributeSet::UAetherAttributeSet()
	: Health(100.0f)
	  , MaxHealth(100.0f)
{
}

void UAetherAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, Atk, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, Def, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, CritRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, CritDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, EnergyRecharge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, ElementalMastery, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, PyroDamageBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, HydroDamageBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, ElectroDamageBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, AnemoDamageBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, CryoDamageBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, GeoDamageBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, PyroAura, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, HydroAura, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, ElectroAura, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, AnemoAura, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, CryoAura, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAetherAttributeSet, GeoAura, COND_None, REPNOTIFY_Always);
}

void UAetherAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetPyroAuraAttribute()
		|| Attribute == GetHydroAuraAttribute()
		|| Attribute == GetElectroAuraAttribute()
		|| Attribute == GetAnemoAuraAttribute()
		|| Attribute == GetCryoAuraAttribute()
		|| Attribute == GetGeoAuraAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 4.0f);
	}
}

void UAetherAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float DamageDone = GetIncomingDamage();
		SetIncomingDamage(0.0f);
		
		if (DamageDone > 0.f)
		{
			SetHealth(FMath::Clamp(GetHealth() - DamageDone, 0.f, GetMaxHealth()));
            
			if (GetHealth() <= 0.f)
			{
				// 사망 처리
			}
		}
		
	}
}

void UAetherAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, Health, OldValue);
}

void UAetherAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, MaxHealth, OldValue);
}

void UAetherAttributeSet::OnRep_Atk(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, Atk, OldValue);
}

void UAetherAttributeSet::OnRep_Def(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, Def, OldValue);
}

void UAetherAttributeSet::OnRep_CritRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, CritRate, OldValue);
}

void UAetherAttributeSet::OnRep_CritDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, CritDamage, OldValue);
}

void UAetherAttributeSet::OnRep_EnergyRecharge(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, EnergyRecharge, OldValue);
}

void UAetherAttributeSet::OnRep_ElementalMastery(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, ElementalMastery, OldValue);
}

void UAetherAttributeSet::OnRep_PyroDamageBonus(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, PyroDamageBonus, OldValue);
}

void UAetherAttributeSet::OnRep_HydroDamageBonus(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, HydroDamageBonus, OldValue);
}

void UAetherAttributeSet::OnRep_ElectroDamageBonus(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, ElectroDamageBonus, OldValue);
}

void UAetherAttributeSet::OnRep_AnemoDamageBonus(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, AnemoDamageBonus, OldValue);
}

void UAetherAttributeSet::OnRep_CryoDamageBonus(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, CryoDamageBonus, OldValue);
}

void UAetherAttributeSet::OnRep_GeoDamageBonus(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, GeoDamageBonus, OldValue);
}

void UAetherAttributeSet::OnRep_PyroAura(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, PyroAura, OldValue);
}

void UAetherAttributeSet::OnRep_HydroAura(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, HydroAura, OldValue);
}

void UAetherAttributeSet::OnRep_ElectroAura(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, ElectroAura, OldValue);
}

void UAetherAttributeSet::OnRep_AnemoAura(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, AnemoAura, OldValue);
}

void UAetherAttributeSet::OnRep_CryoAura(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, CryoAura, OldValue);
}

void UAetherAttributeSet::OnRep_GeoAura(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAetherAttributeSet, GeoAura, OldValue);
}
