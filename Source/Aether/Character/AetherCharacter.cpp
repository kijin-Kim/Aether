// Fill out your copyright notice in the Description page of Project Settings.

// AetherCharacter.cpp
#include "AetherCharacter.h"

#include "Aether/AetherCharacterDatabase.h"
#include "Aether/AbilitySystem/AetherCharacterData.h"
#include "Aether/AetherGameplayTags.h"
#include "Aether/Input/AetherInputComponent.h"
#include "Aether/Input/AetherInputConfig.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"

AAetherCharacter::AAetherCharacter()
{
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());
	SpringArmComponent->TargetArmLength = 400.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraComponent->SetupAttachment(SpringArmComponent);
}

void AAetherCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UAetherCharacterData* CharacterData =
		GetGameInstance()->GetSubsystem<UAetherCharacterDatabase>()->GetCharacterByID(CharacterId);
	if (!CharacterData)
	{
		return;
	}

	UAetherInputConfig* InputConfig = CharacterData->InputConfig.LoadSynchronous();
	if (!InputConfig)
	{
		return;
	}

	UAetherInputComponent* AetherInputComponent = CastChecked<UAetherInputComponent>(PlayerInputComponent);
	AetherInputComponent->BindAbilityInputAction(
		InputConfig, this,
		&AAetherCharacter::AbilityInputPressed,
		&AAetherCharacter::AbilityInputReleased);
	AetherInputComponent->BindInputAction(
		InputConfig, AetherGameplayTags::InputTag_Move,
		ETriggerEvent::Triggered, this, &AAetherCharacter::Move);
	AetherInputComponent->BindInputAction(
		InputConfig, AetherGameplayTags::InputTag_Look,
		ETriggerEvent::Triggered, this, &AAetherCharacter::Look);
}

void AAetherCharacter::AbilityInputPressed(FGameplayTag InputTag)
{
	GetAetherAbilitySystemComponent()->AbilityInputPressed(InputTag);
}

void AAetherCharacter::AbilityInputReleased(FGameplayTag InputTag)
{
	GetAetherAbilitySystemComponent()->AbilityInputReleased(InputTag);
}

void AAetherCharacter::Move(const FInputActionValue& InputActionValue)
{
	FVector2D InputVector = InputActionValue.Get<FVector2D>();
	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(ForwardDirection, InputVector.Y);
	AddMovementInput(RightDirection, InputVector.X);
}

void AAetherCharacter::Look(const FInputActionValue& InputActionValue)
{
	FVector2D InputVector = InputActionValue.Get<FVector2D>();
	AddControllerPitchInput(InputVector.Y);
	AddControllerYawInput(InputVector.X);
}