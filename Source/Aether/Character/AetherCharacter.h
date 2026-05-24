// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Aether/Character/AetherCharacterBase.h"
#include "InputActionValue.h"
#include "AetherCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UAetherInputConfig;
struct FGameplayTag;

UCLASS()
class AETHER_API AAetherCharacter : public AAetherCharacterBase
{
	GENERATED_BODY()

public:
	AAetherCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

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
};
