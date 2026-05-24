// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Aether/Character/AetherCharacterBase.h"
#include "AetherAICharacter.generated.h"

class AAIController;

UCLASS()
class AETHER_API AAetherAICharacter : public AAetherCharacterBase
{
	GENERATED_BODY()

public:
	AAetherAICharacter();

public:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether|AI", meta = (AllowPrivateAccess = "true"))
	FName AssignedCharacterId = NAME_None;
};
