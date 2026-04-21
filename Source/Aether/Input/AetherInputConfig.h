// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AetherInputConfig.generated.h"


class UInputAction;

USTRUCT(BlueprintType)
struct FAetherInputAction
{
	 GENERATED_BODY()
	
	
public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<const UInputAction> InputAction;
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;
};

/**
 * 
 */
UCLASS(BlueprintType, Const)
class AETHER_API UAetherInputConfig : public UDataAsset
{
	GENERATED_BODY()


public:
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = "InputAction"))
	TArray<FAetherInputAction> InputActions;
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = "InputAction"))
	TArray<FAetherInputAction> AbilityInputActions;
};
