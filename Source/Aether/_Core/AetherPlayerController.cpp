// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPlayerController.h"

#include "AbilitySystemComponent.h"
#include "AetherAbilitySystemComponent.h"
#include "AetherPlayerState.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerState.h"

void AAetherPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		check(DefaultInputMappingContext)
		Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
	}
}

void AAetherPlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);
	APlayerState* PS = GetPlayerState<APlayerState>();
	if (!PS)
	{
		return;
	}
	if (UAbilitySystemComponent* ASC = PS->GetComponentByClass<UAbilitySystemComponent>())
	{
		ASC->InitAbilityActorInfo(PS, this);
	}
}

void AAetherPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);
	if (AAetherPlayerState* AetherPS = GetPlayerState<AAetherPlayerState>())
	{
		if (UAetherAbilitySystemComponent* AetherASC = AetherPS->GetAetherAbilitySystemComponent())
		{
			AetherASC->ProcessInputs();
		}
	}
}
