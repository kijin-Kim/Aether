// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AetherPlayerController.generated.h"

class UInputMappingContext;
/**
 * 
 */
UCLASS()
class AETHER_API AAetherPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual auto BeginPlay() -> void override;
	
private:
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;
	
	
};
