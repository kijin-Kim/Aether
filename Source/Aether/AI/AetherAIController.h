// Fill out your copyright notice in the Description page of Project Settings.



#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "AetherAIController.generated.h"

class UAetherAbilitySystemComponent;
class UStateTreeAIComponent;
class AAetherCharacterBase;

UCLASS()
class AETHER_API AAetherAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAetherAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION(BlueprintCallable, Category = "Aether|AI")
	void TryActivateAbilityByTag(FGameplayTag InputTag);

protected:
	UPROPERTY(Transient)
	TObjectPtr<UAetherAbilitySystemComponent> CachedASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;
};