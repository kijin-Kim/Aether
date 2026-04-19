// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPlayerController.h"

#include "AbilitySystemComponent.h"
#include "AetherAbilitySystemComponent.h"
#include "AetherCharacter.h"
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


void AAetherPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);
	if (AAetherCharacter* AetherCharacter = GetPawn<AAetherCharacter>())
	{
		if (UAetherAbilitySystemComponent* AetherASC = AetherCharacter->GetAetherAbilitySystemComponent())
		{
			AetherASC->ProcessInputs();
		}	
	}
}

void AAetherPlayerController::OnPossess(APawn* InPawn)
{
	AAetherPlayerState* AetherPS = GetPlayerState<AAetherPlayerState>();
	AetherPS->AuthInitializeParty(CastChecked<AAetherCharacter>(InPawn));
	Super::OnPossess(InPawn);
}
