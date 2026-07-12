// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AttributeSetCommon.h"
#include "AetherPartyAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class AETHER_API UAetherPartyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	ATTRIBUTE_ACCESSORS(UAetherPartyAttributeSet, Stamina)
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "PartyStats")
	FGameplayAttributeData Stamina;
	
};
