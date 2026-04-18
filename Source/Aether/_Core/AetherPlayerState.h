// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AetherPlayerState.generated.h"

class UAetherAbilitySystemComponent;
class UAttributeSet;
/**
 * 
 */
UCLASS()
class AETHER_API AAetherPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AAetherPlayerState();
	UFUNCTION(BlueprintCallable, Category = "Aether|PlayerState")
	UAetherAbilitySystemComponent* GetAetherAbilitySystemComponent() const { return AetherASC; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	
private:
	UPROPERTY()
	TObjectPtr<UAetherAbilitySystemComponent> AetherASC;
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
};
