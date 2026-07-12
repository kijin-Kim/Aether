// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AetherFieldCharacterPawn.generated.h"

class UAetherAbilitySystemComponent;
struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class AETHER_API AAetherFieldCharacterPawn : public ACharacter
{
	GENERATED_BODY()

public:
	AAetherFieldCharacterPawn();

private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;
};
