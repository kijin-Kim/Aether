// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPlayerController.h"

#include "EnhancedInputSubsystems.h"

void AAetherPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		check(DefaultInputMappingContext)
		Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
	}
	
}

