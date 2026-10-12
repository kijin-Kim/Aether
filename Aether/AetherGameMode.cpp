// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherGameMode.h"
#include "Aether/PartySystem/AetherFieldCharacterPawn.h"

AAetherGameMode::AAetherGameMode()
{
	DefaultPawnClass = AAetherFieldCharacterPawn::StaticClass();
}
