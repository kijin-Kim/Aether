#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "AetherPartySetupComponent.generated.h"

class AAetherFieldCharacterPawn;
class AAetherPartyPresentationActor;
class AAetherPlayerController;
class UAetherPartyComponent;
class UAetherPartySetupWidget;
class UAetherPrimaryGameLayout;
class UCommonActivatableWidget;
class ULevelStreamingDynamic;
class UWorld;

UENUM(BlueprintType)
enum class EAetherPartySetupFlowState : uint8
{
	Closed,
	Overview,
	Selecting
};

UENUM(BlueprintType)
enum class EAetherPartyPresentationState : uint8
{
	Idle,
	Loading,
	Active,
	Closing,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAetherPartySelectionCompleted, bool, bSucceeded);

UCLASS(ClassGroup=(UI), meta=(BlueprintSpawnableComponent))
class AETHER_API UAetherPartySetupComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherPartySetupComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void ConfigurePartySetup(
		TSubclassOf<UCommonActivatableWidget> InPartySelectWidgetClass,
		TSubclassOf<UAetherPartySetupWidget> InPartySetupWidgetClass,
		const TSoftObjectPtr<UWorld>& InPartyPresentationLevel,
		const FTransform& InPartyPresentationLevelTransform,
		float InPartyPresentationCameraBlendTime);
	void InitializePartySetup(UAetherPrimaryGameLayout* InPrimaryGameLayout);
	void ShutdownPartySetup();

	UCommonActivatableWidget* OpenPartySelectUI();
	UAetherPartySetupWidget* OpenPartySetupUI(int32 PartySlotIndex);
	bool SetPartySelectionPreview(FPrimaryAssetId CharacterId);
	bool ConfirmPartySelection();

	int32 GetSelectedPartySlotIndex() const { return SelectedPartySlotIndex; }
	FPrimaryAssetId GetPartySelectionPreviewCharacterId() const { return PartySelectionPreviewCharacterId; }
	bool IsPartySelectionPending() const
	{
		return bPartySelectionPending;
	}
	UFUNCTION(BlueprintPure, Category = "Aether|UI")
	EAetherPartySetupFlowState GetFlowState() const
	{
		if (ActivePartySetupWidget)
		{
			return EAetherPartySetupFlowState::Selecting;
		}
		return ActivePartySelectWidget
			? EAetherPartySetupFlowState::Overview
			: EAetherPartySetupFlowState::Closed;
	}

	UFUNCTION(BlueprintPure, Category = "Aether|UI")
	EAetherPartyPresentationState GetPresentationState() const { return PresentationState; }

	UPROPERTY(BlueprintAssignable, Category = "Aether|UI")
	FOnAetherPartySelectionCompleted OnPartySelectionCompleted;

private:
	AAetherPlayerController* GetAetherController() const;
	UAetherPartyComponent* GetPartyComponent() const;
	void HandleGameMenuDisplayedWidgetChanged(UCommonActivatableWidget* DisplayedWidget);
	void HandlePartyPresentationBlendComplete(uint32 RequestId);
	void HandlePartyPresentationExitBlendComplete(uint32 RequestId);
	void HandlePartyPresentationLoadTimeout();
	void RemovePartyPresentationCameraBlendDelegates();
	void ReleasePartyPresentationStreamingSource();
	void FinishPartySetupFlow();
	void ResetPartySelection(bool bForce = false);
	bool BeginPartyPresentation();
	void EndPartyPresentation();
	bool ActivatePartyPresentation();
	void FailPartyPresentation();
	void RestoreViewTargetImmediately();
	void UnloadPartyPresentationLevel();
	void ClearPartyPresentationLoadTimeout();
	void PauseGameIfNeeded();
	void UnpauseGameIfNeeded();

	UFUNCTION()
	void HandlePartySelectionCompleted(bool bSucceeded);

	UFUNCTION()
	void HandlePresentedPartyChanged();

	UFUNCTION()
	void HandlePartyPresentationLevelShown();

	UPROPERTY()
	TObjectPtr<UAetherPrimaryGameLayout> PrimaryGameLayout;

	UPROPERTY()
	TSubclassOf<UCommonActivatableWidget> PartySelectWidgetClass;

	UPROPERTY()
	TSubclassOf<UAetherPartySetupWidget> PartySetupWidgetClass;

	UPROPERTY()
	TSoftObjectPtr<UWorld> PartyPresentationLevel;

	UPROPERTY()
	FTransform PartyPresentationLevelTransform;

	UPROPERTY()
	float PartyPresentationCameraBlendTime = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Aether|UI", meta = (ClampMin = "1.0"))
	float PartyPresentationLoadTimeout = 15.f;

	UPROPERTY()
	TObjectPtr<UCommonActivatableWidget> ActivePartySelectWidget;

	UPROPERTY()
	TObjectPtr<UAetherPartySetupWidget> ActivePartySetupWidget;

	UPROPERTY()
	TObjectPtr<AAetherPartyPresentationActor> PartyPresentationActor;

	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> PartyPresentationStreamingLevel;

	UPROPERTY()
	TObjectPtr<AActor> PreviousViewTarget;

	TWeakObjectPtr<AAetherFieldCharacterPawn> PartyPresentationFieldPawn;
	FPrimaryAssetId PartySelectionPreviewCharacterId;
	FTimerHandle PartyPresentationLoadTimeoutHandle;
	FDelegateHandle PartyPresentationBlendCompleteHandle;
	FDelegateHandle PartyPresentationExitBlendCompleteHandle;
	int32 SelectedPartySlotIndex = INDEX_NONE;
	EAetherPartyPresentationState PresentationState = EAetherPartyPresentationState::Idle;
	uint32 PartyPresentationRequestId = 0;
	bool bPartySetupPausedGame = false;
	bool bPartySelectionPending = false;
};
