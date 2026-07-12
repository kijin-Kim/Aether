// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherFieldCharacterPawn.h"

#include "Aether/Input/AetherInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"


AAetherFieldCharacterPawn::AAetherFieldCharacterPawn()
{
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());
	SpringArmComponent->TargetArmLength = 400.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraComponent->SetupAttachment(SpringArmComponent);
	OverrideInputComponentClass = UAetherInputComponent::StaticClass();
}
