// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AetherPlayerController.generated.h"

class UInputMappingContext;
class UAetherPartyComponent;
/**
 * 
 */
UCLASS()
class AETHER_API AAetherPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AAetherPlayerController();

	virtual void BeginPlay() override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	UAetherPartyComponent* GetPartyComponent() const { return PartyComponent; }
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|Party", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherPartyComponent> PartyComponent;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;
	
	
};
