// Fill out your copyright notice in the Description page of Project Settings.

#include "AetherPrimaryGameLayout.h"

#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Aether/AetherGameplayTags.h"

UCommonActivatableWidget* UAetherPrimaryGameLayout::PushWidgetToLayer(
	FGameplayTag LayerTag,
	TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		return nullptr;
	}

	UCommonActivatableWidgetStack* LayerStack = GetLayerStack(LayerTag);
	if (!LayerStack)
	{
		return nullptr;
	}

	return LayerStack->AddWidget(WidgetClass);
}

void UAetherPrimaryGameLayout::DeactivateTopWidgetInLayer(FGameplayTag LayerTag)
{
	UCommonActivatableWidgetStack* LayerStack = GetLayerStack(LayerTag);
	if (!LayerStack)
	{
		return;
	}

	if (UCommonActivatableWidget* ActiveWidget = LayerStack->GetActiveWidget())
	{
		ActiveWidget->DeactivateWidget();
	}
}

UCommonActivatableWidgetStack* UAetherPrimaryGameLayout::GetLayerStack(FGameplayTag LayerTag) const
{
	if (LayerTag == AetherGameplayTags::UI_Layer_Game)
	{
		return GameLayerStack;
	}

	if (LayerTag == AetherGameplayTags::UI_Layer_GameMenu)
	{
		return GameMenuLayerStack;
	}

	if (LayerTag == AetherGameplayTags::UI_Layer_Modal)
	{
		return ModalLayerStack;
	}

	return nullptr;
}
