// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "ElementalReactiveInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UElementalReactiveInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AETHER_API IElementalReactiveInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void ReceiveElementalAttack(AActor* SourceActor, FGameplayTag ElementTypeTag, float Gauge, float BaseDamage);
};
