// Fill out your copyright notice in the Description page of Project Settings.

#include "AetherUIManagerSubsystem.h"

#include "AetherPrimaryGameLayout.h"

void UAetherUIManagerSubsystem::RegisterPrimaryGameLayout(UAetherPrimaryGameLayout* InPrimaryGameLayout)
{
	PrimaryGameLayout = InPrimaryGameLayout;
}

void UAetherUIManagerSubsystem::UnregisterPrimaryGameLayout(UAetherPrimaryGameLayout* InPrimaryGameLayout)
{
	if (PrimaryGameLayout.Get() == InPrimaryGameLayout)
	{
		PrimaryGameLayout.Reset();
	}
}

UCommonActivatableWidget* UAetherUIManagerSubsystem::PushWidgetToLayer(
	FGameplayTag LayerTag,
	TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (UAetherPrimaryGameLayout* Layout = PrimaryGameLayout.Get())
	{
		return Layout->PushWidgetToLayer(LayerTag, WidgetClass);
	}

	return nullptr;
}

void UAetherUIManagerSubsystem::DeactivateTopWidgetInLayer(FGameplayTag LayerTag)
{
	if (UAetherPrimaryGameLayout* Layout = PrimaryGameLayout.Get())
	{
		Layout->DeactivateTopWidgetInLayer(LayerTag);
	}
}
