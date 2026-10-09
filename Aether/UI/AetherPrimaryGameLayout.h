// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CommonUserWidget.h"
#include "AetherPrimaryGameLayout.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;

UCLASS(Abstract, BlueprintType, Blueprintable)
class AETHER_API UAetherPrimaryGameLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Aether|UI")
	UCommonActivatableWidget* PushWidgetToLayer(FGameplayTag LayerTag,
	                                           TSubclassOf<UCommonActivatableWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "Aether|UI")
	void DeactivateTopWidgetInLayer(FGameplayTag LayerTag);

	UCommonActivatableWidgetStack* GetLayerStack(FGameplayTag LayerTag) const;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UCommonActivatableWidgetStack> GameLayerStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UCommonActivatableWidgetStack> GameMenuLayerStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UCommonActivatableWidgetStack> ModalLayerStack;
};
