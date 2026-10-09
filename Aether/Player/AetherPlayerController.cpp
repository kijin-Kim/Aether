// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPlayerController.h"

#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "AetherPlayerState.h"
#include "EnhancedInputSubsystems.h"
#include "Aether/AetherGameplayTags.h"
#include "Aether/Input/AetherInputComponent.h"
#include "Aether/UI/AetherPrimaryGameLayout.h"
#include "Aether/UI/AetherUIManagerSubsystem.h"

AAetherPlayerController::AAetherPlayerController()
{
	PartySetupComponent = CreateDefaultSubobject<UAetherPartySetupComponent>(TEXT("PartySetupComponent"));
	PartyPresentationLevel = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Aether/UI/Party/PartyPresentationLevel.PartyPresentationLevel")));
}

void AAetherPlayerController::BeginPlay()
{
	Super::BeginPlay();

	InitializePrimaryGameLayout();
	if (PartySetupComponent)
	{
		PartySetupComponent->ConfigurePartySetup(
			PartySelectWidgetClass,
			PartySetupWidgetClass,
			PartyPresentationLevel,
			PartyPresentationLevelTransform,
			PartyPresentationCameraBlendTime);
		PartySetupComponent->OnPartySelectionCompleted.AddUniqueDynamic(
			this, &AAetherPlayerController::HandlePartySelectionCompletedForwarded);
		PartySetupComponent->InitializePartySetup(PrimaryGameLayout);
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer()))
	{
		check(DefaultInputMappingContext)
		Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
	}
}

void AAetherPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PartySetupComponent)
	{
		PartySetupComponent->OnPartySelectionCompleted.RemoveDynamic(
			this, &AAetherPlayerController::HandlePartySelectionCompletedForwarded);
		PartySetupComponent->ShutdownPartySetup();
	}

	if (PrimaryGameLayout)
	{
		if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UAetherUIManagerSubsystem* UIManager =
				LocalPlayer->GetSubsystem<UAetherUIManagerSubsystem>())
			{
				UIManager->UnregisterPrimaryGameLayout(PrimaryGameLayout);
			}
		}
		PrimaryGameLayout->RemoveFromParent();
		PrimaryGameLayout = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AAetherPlayerController::InitializePrimaryGameLayout()
{
	if (!PrimaryGameLayoutClass || PrimaryGameLayout)
	{
		return;
	}

	PrimaryGameLayout = CreateWidget<UAetherPrimaryGameLayout>(this, PrimaryGameLayoutClass);
	if (!PrimaryGameLayout)
	{
		return;
	}

	PrimaryGameLayout->AddToViewport();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UAetherUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UAetherUIManagerSubsystem>())
		{
			UIManager->RegisterPrimaryGameLayout(PrimaryGameLayout);
		}
	}

}

UCommonActivatableWidget* AAetherPlayerController::OpenPartySelectUI()
{
	return PartySetupComponent ? PartySetupComponent->OpenPartySelectUI() : nullptr;
}

UAetherPartySetupWidget* AAetherPlayerController::OpenPartySetupUI(const int32 PartySlotIndex)
{
	return PartySetupComponent ? PartySetupComponent->OpenPartySetupUI(PartySlotIndex) : nullptr;
}

bool AAetherPlayerController::SetPartySelectionPreview(const FPrimaryAssetId CharacterId)
{
	return PartySetupComponent && PartySetupComponent->SetPartySelectionPreview(CharacterId);
}

bool AAetherPlayerController::ConfirmPartySelection()
{
	return PartySetupComponent && PartySetupComponent->ConfirmPartySelection();
}

int32 AAetherPlayerController::GetSelectedPartySlotIndex() const
{
	return PartySetupComponent ? PartySetupComponent->GetSelectedPartySlotIndex() : INDEX_NONE;
}

FPrimaryAssetId AAetherPlayerController::GetPartySelectionPreviewCharacterId() const
{
	return PartySetupComponent
		? PartySetupComponent->GetPartySelectionPreviewCharacterId()
		: FPrimaryAssetId();
}

bool AAetherPlayerController::IsPartySelectionPending() const
{
	return PartySetupComponent && PartySetupComponent->IsPartySelectionPending();
}

void AAetherPlayerController::HandlePartySelectionCompletedForwarded(const bool bSucceeded)
{
	OnPartySelectionCompleted.Broadcast(bSucceeded);
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
		AetherInputComponent->BindInputAction(
			DefaultInputConfig, AetherGameplayTags::InputTag_UI_PartySetup,
			ETriggerEvent::Completed, this, &AAetherPlayerController::HandlePartySelectInputCompleted);
	}
}

void AAetherPlayerController::HandlePartySelectInputCompleted()
{
	OpenPartySelectUI();
}

void AAetherPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);
	if (AAetherPlayerState* AetherPlayerState = GetPlayerState<AAetherPlayerState>())
	{
		AetherPlayerState->ProcessInputs();
	}
}

void AAetherPlayerController::AbilityInputPressed(FGameplayTag InputTag)
{
	AAetherPlayerState* AetherPlayerState = GetPlayerState<AAetherPlayerState>();
	if (!AetherPlayerState)
	{
		return;
	}

	if (UAetherAbilitySystemComponent* PartyASC = AetherPlayerState->GetPartyASC())
	{
		PartyASC->AbilityInputPressed(InputTag);
	}
	if (UAetherAbilitySystemComponent* ActiveASC = AetherPlayerState->GetActiveAbilitySystemComponent())
	{
		ActiveASC->AbilityInputPressed(InputTag);
	}
}

void AAetherPlayerController::AbilityInputReleased(FGameplayTag InputTag)
{
	AAetherPlayerState* AetherPlayerState = GetPlayerState<AAetherPlayerState>();
	if (!AetherPlayerState)
	{
		return;
	}

	if (UAetherAbilitySystemComponent* PartyASC = AetherPlayerState->GetPartyASC())
	{
		PartyASC->AbilityInputReleased(InputTag);
	}
	if (UAetherAbilitySystemComponent* ActiveASC = AetherPlayerState->GetActiveAbilitySystemComponent())
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
