// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "AetherCharacter.generated.h"

struct FGameplayTag;
class UAetherAbilitySet;
class UAetherInputConfig;
class UGameplayAbility;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class AETHER_API AAetherCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAetherCharacter();
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	
	

private:
	void AbilityInputPressed(FGameplayTag InputTag);
	void AbilityInputReleased(FGameplayTag InputTag);
	
	void Move(const FInputActionValue& InputActionValue);
	void Look(const FInputActionValue& InputActionValue);
	

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;
	
	
	UPROPERTY(EditDefaultsOnly, Category = "Aether|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<const UAetherInputConfig> InputConfig;
	UPROPERTY(EditDefaultsOnly, Category = "Aether|Abilities", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<const UAetherAbilitySet> AbilitySet;
};
