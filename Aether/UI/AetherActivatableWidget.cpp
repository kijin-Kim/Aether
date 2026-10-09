// Fill out your copyright notice in the Description page of Project Settings.

#include "AetherActivatableWidget.h"

#include "CommonInputModeTypes.h"
#include "Input/UIActionBindingHandle.h"
#include "UObject/ConstructorHelpers.h"


TOptional<FUIInputConfig> UAetherActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputMode)
	{
	case EAetherWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case EAetherWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case EAetherWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case EAetherWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}
