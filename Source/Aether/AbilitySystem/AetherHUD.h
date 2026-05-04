// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AetherHUD.generated.h"

/**
 * 
 */
UCLASS()
class AETHER_API AAetherHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;
};
