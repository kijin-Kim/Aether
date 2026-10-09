// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "Aether/UI/AetherPartySetupComponent.h"
#include "AetherPlayerController.generated.h"

class UAetherInputConfig;
class UAetherPartySetupWidget;
class UAetherPrimaryGameLayout;
class UInputMappingContext;
class UCommonActivatableWidget;

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
	virtual void SetupInputComponent() override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void AbilityInputPressed(FGameplayTag InputTag);
	void AbilityInputReleased(FGameplayTag InputTag);
	void Move(const FInputActionValue& InputActionValue);
	void Look(const FInputActionValue& InputActionValue);

	UFUNCTION(BlueprintCallable, Category = "Aether|UI")
	UCommonActivatableWidget* OpenPartySelectUI();

	UFUNCTION(BlueprintCallable, Category = "Aether|UI")
	UAetherPartySetupWidget* OpenPartySetupUI(int32 PartySlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Aether|UI")
	bool SetPartySelectionPreview(FPrimaryAssetId CharacterId);

	UFUNCTION(BlueprintCallable, Category = "Aether|UI")
	bool ConfirmPartySelection();

	UFUNCTION(BlueprintPure, Category = "Aether|UI")
	int32 GetSelectedPartySlotIndex() const;

	UFUNCTION(BlueprintPure, Category = "Aether|UI")
	FPrimaryAssetId GetPartySelectionPreviewCharacterId() const;

	UFUNCTION(BlueprintPure, Category = "Aether|UI")
	bool IsPartySelectionPending() const;

	UPROPERTY(BlueprintAssignable, Category = "Aether|UI")
	FOnAetherPartySelectionCompleted OnPartySelectionCompleted;

private:
	void InitializePrimaryGameLayout();
	void HandlePartySelectInputCompleted();

	UFUNCTION()
	void HandlePartySelectionCompletedForwarded(bool bSucceeded);

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherInputConfig> DefaultInputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Aether|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAetherPrimaryGameLayout> PrimaryGameLayoutClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aether|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCommonActivatableWidget> PartySelectWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aether|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAetherPartySetupWidget> PartySetupWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aether|UI", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> PartyPresentationLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Aether|UI", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float PartyPresentationCameraBlendTime = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Aether|UI", meta = (AllowPrivateAccess = "true"))
	FTransform PartyPresentationLevelTransform =
		FTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, 100000.f));

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherPartySetupComponent> PartySetupComponent;

	UPROPERTY()
	TObjectPtr<UAetherPrimaryGameLayout> PrimaryGameLayout;
};
