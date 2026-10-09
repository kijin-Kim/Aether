// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "AetherUIManagerSubsystem.generated.h"

class UAetherPrimaryGameLayout;
class UCommonActivatableWidget;

UCLASS()
class AETHER_API UAetherUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Aether|UI")
	void RegisterPrimaryGameLayout(UAetherPrimaryGameLayout* InPrimaryGameLayout);

	UFUNCTION(BlueprintCallable, Category = "Aether|UI")
	void UnregisterPrimaryGameLayout(UAetherPrimaryGameLayout* InPrimaryGameLayout);

	UFUNCTION(BlueprintPure, Category = "Aether|UI")
	UAetherPrimaryGameLayout* GetPrimaryGameLayout() const { return PrimaryGameLayout.Get(); }

	UFUNCTION(BlueprintPure, Category = "Aether|UI")
	bool IsPrimaryGameLayoutRegistered() const { return PrimaryGameLayout.IsValid(); }

	UFUNCTION(BlueprintCallable, Category = "Aether|UI")
	UCommonActivatableWidget* PushWidgetToLayer(FGameplayTag LayerTag,
	                                           TSubclassOf<UCommonActivatableWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "Aether|UI")
	void DeactivateTopWidgetInLayer(FGameplayTag LayerTag);

private:
	TWeakObjectPtr<UAetherPrimaryGameLayout> PrimaryGameLayout;
};
