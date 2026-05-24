// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherAICharacter.h"

#include "Aether/Aether.h"

AAetherAICharacter::AAetherAICharacter()
{
	bOnField = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AAetherAICharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AssignedCharacterId.IsNone())
	{
		UE_LOG(LogAether, Warning, TEXT("AAetherAICharacter %s has no AssignedCharacterId. Skipping data init."), *GetName());
		return;
	}

	InitializeFromCharacterData(AssignedCharacterId);
}