// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPlayerController.h"

#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "AetherPlayerState.h"
#include "EnhancedInputSubsystems.h"
#include "Aether/AetherGameplayTags.h"
#include "Aether/Input/AetherInputComponent.h"


void AAetherPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer()))
	{
		check(DefaultInputMappingContext)
		Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
	}
}

void AAetherPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (DefaultInputConfig)
	{
		UAetherInputComponent* AetherInputComponent = CastChecked<UAetherInputComponent>(InputComponent);
		AetherInputComponent->BindAbilityInputAction(
			DefaultInputConfig, this,
			&AAetherPlayerController::AbilityInputPressed,
			&AAetherPlayerController::AbilityInputReleased);

		AetherInputComponent->BindInputAction(
			DefaultInputConfig, AetherGameplayTags::InputTag_Move,
			ETriggerEvent::Triggered, this, &AAetherPlayerController::Move);
		AetherInputComponent->BindInputAction(
			DefaultInputConfig, AetherGameplayTags::InputTag_Look,
			ETriggerEvent::Triggered, this, &AAetherPlayerController::Look);
	}
}

void AAetherPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);
	GetPlayerState<AAetherPlayerState>()->ProcessInputs();
}

void AAetherPlayerController::AbilityInputPressed(FGameplayTag InputTag)
{
	if (UAetherAbilitySystemComponent* PartyASC = Cast<IAetherPartyAbilitySystemInterface>(PlayerState)->
		GetPartyAbilitySystemComponent())
	{
		PartyASC->AbilityInputPressed(InputTag);
	}
	if (UAetherAbilitySystemComponent* ActiveASC = Cast<IAetherPartyAbilitySystemInterface>(PlayerState)->

		GetActiveAbilitySystemComponent())
	{
		ActiveASC->AbilityInputPressed(InputTag);
	}
}

void AAetherPlayerController::AbilityInputReleased(FGameplayTag InputTag)
{
	if (UAetherAbilitySystemComponent* PartyASC = Cast<IAetherPartyAbilitySystemInterface>(PlayerState)->
		GetPartyAbilitySystemComponent())
	{
		PartyASC->AbilityInputReleased(InputTag);
	}
	if (UAetherAbilitySystemComponent* ActiveASC = Cast<IAetherPartyAbilitySystemInterface>(PlayerState)->

		GetActiveAbilitySystemComponent())
	{
		ActiveASC->AbilityInputReleased(InputTag);
	}
}

void AAetherPlayerController::Move(const FInputActionValue& InputActionValue)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		FVector2D InputVector = InputActionValue.Get<FVector2D>();
		const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		ControlledPawn->AddMovementInput(ForwardDirection, InputVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputVector.X);
	}
}

void AAetherPlayerController::Look(const FInputActionValue& InputActionValue)
{
	if (GetPawn())
	{
		FVector2D InputVector = InputActionValue.Get<FVector2D>();

		AddPitchInput(InputVector.Y);
		AddYawInput(InputVector.X);
	}
}
