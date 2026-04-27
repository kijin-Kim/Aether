// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherBaseAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"


UAetherBaseAttributeSet::UAetherBaseAttributeSet()
	: Level(1.0f)
	  , Health(100.0f)
	  , MaxHealth(100.0f)
	  , Atk(41.0f)
{
}

void UAetherBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
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

void UAetherBaseAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
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
