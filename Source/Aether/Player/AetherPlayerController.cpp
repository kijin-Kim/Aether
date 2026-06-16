// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPlayerController.h"

#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/Character/AetherCharacter.h"
#include "AetherPartyComponent.h"
#include "EnhancedInputSubsystems.h"

AAetherPlayerController::AAetherPlayerController()
{
	PartyComponent = CreateDefaultSubobject<UAetherPartyComponent>("PartyComponent");
}

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

