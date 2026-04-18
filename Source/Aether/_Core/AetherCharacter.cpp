// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherCharacter.h"

#include "AbilitySystemComponent.h"
#include "AetherAbilitySet.h"
#include "AetherAbilitySystemComponent.h"
#include "AetherGameplayTags.h"
#include "AetherInputComponent.h"
#include "AetherPlayerState.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"


AAetherCharacter::AAetherCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationYaw = false;


	GetCapsuleComponent()->SetCapsuleHalfHeight(90.0f);
	GetCapsuleComponent()->SetCapsuleRadius(35.0f);

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -89.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));


	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());
	SpringArmComponent->TargetArmLength = 400.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraComponent->SetupAttachment(SpringArmComponent);

	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AAetherCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UAetherInputComponent* AetherInputComponent = CastChecked<UAetherInputComponent>(PlayerInputComponent))
	{
		AetherInputComponent->BindAbilityInputAction(InputConfig, this, &AAetherCharacter::AbilityInputPressed, &AAetherCharacter::AbilityInputReleased);
		AetherInputComponent->BindInputAction(InputConfig, AetherGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &AAetherCharacter::Move);
		AetherInputComponent->BindInputAction(InputConfig, AetherGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &AAetherCharacter::Look);
	}
}

void AAetherCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (ensure(AbilitySet))
	{
		if (AAetherPlayerState* AetherPS = GetPlayerState<AAetherPlayerState>())
		{
			AbilitySet->InitializeAbilitySystem(AetherPS->GetAetherAbilitySystemComponent(), nullptr);
		}
	}
}

UAbilitySystemComponent* AAetherCharacter::GetAbilitySystemComponent() const
{
	if (AAetherPlayerState* AetherPS = GetPlayerState<AAetherPlayerState>())
	{
		return AetherPS->GetAbilitySystemComponent();
	}
	return nullptr;
}

void AAetherCharacter::AbilityInputPressed(FGameplayTag InputTag)
{
	if (AAetherPlayerState* AetherPS = GetPlayerState<AAetherPlayerState>())
	{
		if (UAetherAbilitySystemComponent* AetherASC = Cast<UAetherAbilitySystemComponent>(AetherPS->GetAbilitySystemComponent()))
		{
			AetherASC->AbilityInputPressed(InputTag);
		}
	}
}

void AAetherCharacter::AbilityInputReleased(FGameplayTag InputTag)
{
	if (AAetherPlayerState* AetherPS = GetPlayerState<AAetherPlayerState>())
	{
		if (UAetherAbilitySystemComponent* AetherASC = Cast<UAetherAbilitySystemComponent>(AetherPS->GetAbilitySystemComponent()))
		{
			AetherASC->AbilityInputReleased(InputTag);
		}
	}
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
