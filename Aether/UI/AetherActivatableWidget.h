// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AetherActivatableWidget.generated.h"

UENUM(BlueprintType)
enum class EAetherWidgetInputMode : uint8
{
	Default,
	Game,
	Menu,
	GameAndMenu
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class AETHER_API UAetherActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Aether|Input")
	EAetherWidgetInputMode InputMode = EAetherWidgetInputMode::Menu;

	UPROPERTY(EditDefaultsOnly, Category = "Aether|Input")
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
};
