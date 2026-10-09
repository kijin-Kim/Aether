#include "AetherPartySetupComponent.h"

#include "AetherPartyPresentationActor.h"
#include "AetherPrimaryGameLayout.h"
#include "AetherPartySetupWidget.h"
#include "Aether/AetherGameplayTags.h"
#include "Aether/PartySystem/AetherFieldCharacterPawn.h"
#include "Aether/PartySystem/AetherPartyComponent.h"
#include "Aether/Player/AetherPlayerController.h"
#include "Aether/Player/AetherPlayerState.h"
#include "Camera/PlayerCameraManager.h"
#include "CommonActivatableWidget.h"
#include "Engine/Level.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

UAetherPartySetupComponent::UAetherPartySetupComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAetherPartySetupComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ShutdownPartySetup();
	Super::EndPlay(EndPlayReason);
}

void UAetherPartySetupComponent::ConfigurePartySetup(
	TSubclassOf<UCommonActivatableWidget> InPartySelectWidgetClass,
	TSubclassOf<UAetherPartySetupWidget> InPartySetupWidgetClass,
	const TSoftObjectPtr<UWorld>& InPartyPresentationLevel,
	const FTransform& InPartyPresentationLevelTransform,
	const float InPartyPresentationCameraBlendTime)
{
	PartySelectWidgetClass = InPartySelectWidgetClass;
	PartySetupWidgetClass = InPartySetupWidgetClass;
	PartyPresentationLevel = InPartyPresentationLevel;
	PartyPresentationLevelTransform = InPartyPresentationLevelTransform;
	PartyPresentationCameraBlendTime = FMath::Max(0.f, InPartyPresentationCameraBlendTime);
}

void UAetherPartySetupComponent::InitializePartySetup(UAetherPrimaryGameLayout* InPrimaryGameLayout)
{
	if (PrimaryGameLayout && PrimaryGameLayout == InPrimaryGameLayout)
	{
		return;
	}

	ShutdownPartySetup();
	PrimaryGameLayout = InPrimaryGameLayout;
	if (PrimaryGameLayout)
	{
		if (UCommonActivatableWidgetStack* GameMenuStack =
			PrimaryGameLayout->GetLayerStack(AetherGameplayTags::UI_Layer_GameMenu))
		{
			GameMenuStack->OnDisplayedWidgetChanged().AddUObject(
				this, &UAetherPartySetupComponent::HandleGameMenuDisplayedWidgetChanged);
		}
	}
}

void UAetherPartySetupComponent::ShutdownPartySetup()
{
	ClearPartyPresentationLoadTimeout();

	if (PrimaryGameLayout)
	{
		if (UCommonActivatableWidgetStack* GameMenuStack =
			PrimaryGameLayout->GetLayerStack(AetherGameplayTags::UI_Layer_GameMenu))
		{
			GameMenuStack->OnDisplayedWidgetChanged().RemoveAll(this);
		}
	}

	RemovePartyPresentationCameraBlendDelegates();

	if (UAetherPartyComponent* PartyComponent = GetPartyComponent())
	{
		PartyComponent->OnPartyChanged.RemoveDynamic(this, &UAetherPartySetupComponent::HandlePresentedPartyChanged);
	}

	UnpauseGameIfNeeded();
	RestoreViewTargetImmediately();
	UnloadPartyPresentationLevel();

	PrimaryGameLayout = nullptr;
	ActivePartySelectWidget = nullptr;
	ActivePartySetupWidget = nullptr;
	ResetPartySelection(true);
	bPartySelectionPending = false;
	PresentationState = EAetherPartyPresentationState::Idle;
}

UCommonActivatableWidget* UAetherPartySetupComponent::OpenPartySelectUI()
{
	if (!PrimaryGameLayout || !PartySelectWidgetClass)
	{
		return nullptr;
	}

	if (ActivePartySelectWidget)
	{
		return ActivePartySelectWidget;
	}

	UCommonActivatableWidget* PartySelectWidget =
		PrimaryGameLayout->PushWidgetToLayer(AetherGameplayTags::UI_Layer_GameMenu, PartySelectWidgetClass);
	if (!PartySelectWidget)
	{
		return nullptr;
	}

	ActivePartySelectWidget = PartySelectWidget;
	if (!BeginPartyPresentation())
	{
		PauseGameIfNeeded();
	}
	return PartySelectWidget;
}

UAetherPartySetupWidget* UAetherPartySetupComponent::OpenPartySetupUI(const int32 PartySlotIndex)
{
	if (!ActivePartySelectWidget
		|| bPartySelectionPending
		|| !PartySetupWidgetClass)
	{
		return nullptr;
	}

	if (ActivePartySetupWidget)
	{
		return ActivePartySetupWidget;
	}
	if (GetFlowState() != EAetherPartySetupFlowState::Overview)
	{
		return nullptr;
	}

	UAetherPartyComponent* PartyComponent = GetPartyComponent();
	const TArray<FPrimaryAssetId> PartyCharacterIds = PartyComponent
		                                                  ? PartyComponent->GetPartyCharacterIds()
		                                                  : TArray<FPrimaryAssetId>();
	if (!PartyCharacterIds.IsValidIndex(PartySlotIndex))
	{
		return nullptr;
	}

	SelectedPartySlotIndex = PartySlotIndex;
	PartySelectionPreviewCharacterId = PartyCharacterIds[PartySlotIndex];

	UCommonActivatableWidgetStack* GameMenuStack =
		PrimaryGameLayout->GetLayerStack(AetherGameplayTags::UI_Layer_GameMenu);
	UAetherPartySetupWidget* PartySetupWidget = GameMenuStack
		? GameMenuStack->AddWidget<UAetherPartySetupWidget>(
			PartySetupWidgetClass,
			[PartySlotIndex](UAetherPartySetupWidget& Widget)
			{
				Widget.InitializePartySlot(PartySlotIndex);
			})
		: nullptr;
	if (!PartySetupWidget)
	{
		ResetPartySelection(true);
		return nullptr;
	}

	ActivePartySetupWidget = PartySetupWidget;
	if (IsValid(PartyPresentationActor))
	{
		PartyPresentationActor->ShowCharacterPreview(PartySelectionPreviewCharacterId);
	}
	return PartySetupWidget;
}

bool UAetherPartySetupComponent::SetPartySelectionPreview(const FPrimaryAssetId CharacterId)
{
	AAetherPlayerController* Controller = GetAetherController();
	AAetherPlayerState* AetherPlayerState = Controller
		                                        ? Controller->GetPlayerState<AAetherPlayerState>()
		                                        : nullptr;
	if (!ActivePartySetupWidget
		|| GetFlowState() != EAetherPartySetupFlowState::Selecting
		|| bPartySelectionPending
		|| !CharacterId.IsValid()
		|| !AetherPlayerState)
	{
		return false;
	}

	const bool bOwnsCharacter = AetherPlayerState->GetOwnedRoster().ContainsByPredicate(
		[&CharacterId](const FAetherOwnedCharacterData& OwnedCharacter)
		{
			return OwnedCharacter.CharacterId == CharacterId;
		});
	if (!bOwnsCharacter)
	{
		return false;
	}

	PartySelectionPreviewCharacterId = CharacterId;
	if (IsValid(PartyPresentationActor))
	{
		PartyPresentationActor->ShowCharacterPreview(CharacterId);
	}
	return true;
}

bool UAetherPartySetupComponent::ConfirmPartySelection()
{
	AAetherPlayerController* Controller = GetAetherController();
	AAetherPlayerState* AetherPlayerState = Controller
		                                        ? Controller->GetPlayerState<AAetherPlayerState>()
		                                        : nullptr;
	if (!ActivePartySetupWidget
		|| GetFlowState() != EAetherPartySetupFlowState::Selecting
		|| bPartySelectionPending
		|| SelectedPartySlotIndex == INDEX_NONE
		|| !PartySelectionPreviewCharacterId.IsValid()
		|| !AetherPlayerState)
	{
		return false;
	}

	bPartySelectionPending = true;
	FOnAetherPartyMemberReplaceCompleted OnCompleted;
	OnCompleted.BindDynamic(this, &UAetherPartySetupComponent::HandlePartySelectionCompleted);
	return AetherPlayerState->RequestReplacePartyMember(
		SelectedPartySlotIndex,
		PartySelectionPreviewCharacterId,
		MoveTemp(OnCompleted));
}

bool UAetherPartySetupComponent::BeginPartyPresentation()
{
	if (!ActivePartySelectWidget)
	{
		return false;
	}

	if (PresentationState == EAetherPartyPresentationState::Loading)
	{
		return true;
	}
	if (PresentationState == EAetherPartyPresentationState::Failed)
	{
		return false;
	}

	if (PresentationState == EAetherPartyPresentationState::Closing)
	{
		RemovePartyPresentationCameraBlendDelegates();
		RestoreViewTargetImmediately();
		UnloadPartyPresentationLevel();
	}

	if (IsValid(PartyPresentationActor))
	{
		return ActivatePartyPresentation();
	}

	if (PartyPresentationStreamingLevel)
	{
		FailPartyPresentation();
		return false;
	}

	AAetherPlayerController* Controller = GetAetherController();
	UAetherPartyComponent* PartyComponent = GetPartyComponent();
	if (!Controller || !PartyComponent || !GetWorld() || PartyPresentationLevel.IsNull())
	{
		PresentationState = EAetherPartyPresentationState::Failed;
		return false;
	}

	PreviousViewTarget = Controller->GetViewTarget();
	PartyPresentationFieldPawn = Cast<AAetherFieldCharacterPawn>(Controller->GetPawn());
	if (PartyPresentationFieldPawn.IsValid())
	{
		PartyPresentationFieldPawn->SetFieldStreamingSourceEnabled(true);
	}

	bool bLoadRequested = false;
	++PartyPresentationRequestId;
	PresentationState = EAetherPartyPresentationState::Loading;
	PartyPresentationStreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		this,
		PartyPresentationLevel,
		PartyPresentationLevelTransform,
		bLoadRequested);
	if (!bLoadRequested || !PartyPresentationStreamingLevel)
	{
		FailPartyPresentation();
		return false;
	}

	PartyPresentationStreamingLevel->OnLevelShown.AddDynamic(
		this, &UAetherPartySetupComponent::HandlePartyPresentationLevelShown);
	GetWorld()->GetTimerManager().SetTimer(
		PartyPresentationLoadTimeoutHandle,
		this,
		&UAetherPartySetupComponent::HandlePartyPresentationLoadTimeout,
		PartyPresentationLoadTimeout,
		false);
	return true;
}

void UAetherPartySetupComponent::EndPartyPresentation()
{
	ClearPartyPresentationLoadTimeout();
	if (PresentationState == EAetherPartyPresentationState::Idle
		&& !PartyPresentationStreamingLevel
		&& !IsValid(PartyPresentationActor))
	{
		return;
	}

	AAetherPlayerController* Controller = GetAetherController();
	RemovePartyPresentationCameraBlendDelegates();

	if (UAetherPartyComponent* PartyComponent = GetPartyComponent())
	{
		PartyComponent->OnPartyChanged.RemoveDynamic(this, &UAetherPartySetupComponent::HandlePresentedPartyChanged);
	}

	AActor* RestoredViewTarget = IsValid(PreviousViewTarget)
		                             ? PreviousViewTarget.Get()
		                             : Controller
		                             ? Controller->GetPawn()
		                             : nullptr;
	if (Controller && IsValid(PartyPresentationActor) && RestoredViewTarget)
	{
		PresentationState = EAetherPartyPresentationState::Closing;
		const float BlendTime = Controller->IsPaused() ? 0.f : PartyPresentationCameraBlendTime;
		if (BlendTime > 0.f && Controller->PlayerCameraManager)
		{
			PartyPresentationExitBlendCompleteHandle =
				Controller->PlayerCameraManager->OnBlendComplete().AddUObject(
					this,
					&UAetherPartySetupComponent::HandlePartyPresentationExitBlendComplete,
					PartyPresentationRequestId);
		}

		Controller->SetViewTargetWithBlend(
			RestoredViewTarget,
			BlendTime,
			EViewTargetBlendFunction::VTBlend_Cubic,
			0.f,
			true);

		if (BlendTime <= 0.f || !Controller->PlayerCameraManager)
		{
			UnloadPartyPresentationLevel();
		}
	}
	else
	{
		if (Controller && RestoredViewTarget)
		{
			Controller->SetViewTarget(RestoredViewTarget);
		}
		UnloadPartyPresentationLevel();
	}
}

void UAetherPartySetupComponent::HandlePartyPresentationLevelShown()
{
	if (PartyPresentationStreamingLevel)
	{
		PartyPresentationStreamingLevel->OnLevelShown.RemoveDynamic(
			this, &UAetherPartySetupComponent::HandlePartyPresentationLevelShown);
	}
	ClearPartyPresentationLoadTimeout();

	if (!ActivePartySelectWidget || PresentationState != EAetherPartyPresentationState::Loading)
	{
		UnloadPartyPresentationLevel();
		return;
	}

	if (!ActivatePartyPresentation())
	{
		FailPartyPresentation();
	}
}

bool UAetherPartySetupComponent::ActivatePartyPresentation()
{
	AAetherPlayerController* Controller = GetAetherController();
	UAetherPartyComponent* PartyComponent = GetPartyComponent();
	ULevel* LoadedLevel =
		PartyPresentationStreamingLevel ? PartyPresentationStreamingLevel->GetLoadedLevel() : nullptr;
	if (!Controller || !PartyComponent || !LoadedLevel || !GetWorld())
	{
		return false;
	}

	if (!IsValid(PartyPresentationActor))
	{
		for (AActor* LevelActor : LoadedLevel->Actors)
		{
			if (AAetherPartyPresentationActor* PresentationActor =
				Cast<AAetherPartyPresentationActor>(LevelActor))
			{
				PartyPresentationActor = PresentationActor;
				break;
			}
		}
	}

	if (!ensureMsgf(
		IsValid(PartyPresentationActor),
		TEXT("PartyPresentationLevel must contain an AetherPartyPresentationActor.")))
	{
		return false;
	}

	PartyComponent->OnPartyChanged.AddUniqueDynamic(
		this, &UAetherPartySetupComponent::HandlePresentedPartyChanged);
	PartyPresentationActor->SetPartyCharacters(PartyComponent->GetPartyCharacterIds());
	if (ActivePartySetupWidget && PartySelectionPreviewCharacterId.IsValid())
	{
		PartyPresentationActor->ShowCharacterPreview(PartySelectionPreviewCharacterId);
	}
	else
	{
		PartyPresentationActor->ShowPartyOverview();
	}

	RemovePartyPresentationCameraBlendDelegates();

	PresentationState = EAetherPartyPresentationState::Active;
	const float BlendTime = Controller->IsPaused() ? 0.f : PartyPresentationCameraBlendTime;
	if (BlendTime > 0.f && Controller->PlayerCameraManager)
	{
		PartyPresentationBlendCompleteHandle =
			Controller->PlayerCameraManager->OnBlendComplete().AddUObject(
				this,
				&UAetherPartySetupComponent::HandlePartyPresentationBlendComplete,
				PartyPresentationRequestId);
	}

	Controller->SetViewTargetWithBlend(
		PartyPresentationActor,
		BlendTime,
		EViewTargetBlendFunction::VTBlend_Cubic);

	if (!Controller->IsPaused() && (BlendTime <= 0.f || !Controller->PlayerCameraManager))
	{
		PauseGameIfNeeded();
	}
	return true;
}

void UAetherPartySetupComponent::HandlePartyPresentationLoadTimeout()
{
	if (PresentationState == EAetherPartyPresentationState::Loading)
	{
		FailPartyPresentation();
	}
}

void UAetherPartySetupComponent::FailPartyPresentation()
{
	ClearPartyPresentationLoadTimeout();
	RestoreViewTargetImmediately();
	UnloadPartyPresentationLevel();
	PresentationState = EAetherPartyPresentationState::Failed;
	PauseGameIfNeeded();
}

void UAetherPartySetupComponent::RestoreViewTargetImmediately()
{
	if (!IsValid(PreviousViewTarget)
		&& !PartyPresentationStreamingLevel
		&& !IsValid(PartyPresentationActor))
	{
		return;
	}

	AAetherPlayerController* Controller = GetAetherController();
	AActor* RestoredViewTarget = IsValid(PreviousViewTarget)
		                             ? PreviousViewTarget.Get()
		                             : Controller
		                             ? Controller->GetPawn()
		                             : nullptr;
	if (Controller && RestoredViewTarget)
	{
		Controller->SetViewTarget(RestoredViewTarget);
	}
}

void UAetherPartySetupComponent::ReleasePartyPresentationStreamingSource()
{
	if (PartyPresentationFieldPawn.IsValid())
	{
		PartyPresentationFieldPawn->SetFieldStreamingSourceEnabled(false);
	}
	PartyPresentationFieldPawn.Reset();
}

void UAetherPartySetupComponent::HandlePartyPresentationExitBlendComplete(const uint32 RequestId)
{
	AAetherPlayerController* Controller = GetAetherController();
	if (RequestId != PartyPresentationRequestId
		|| PresentationState != EAetherPartyPresentationState::Closing)
	{
		return;
	}

	AActor* RestoredViewTarget = IsValid(PreviousViewTarget)
		                             ? PreviousViewTarget.Get()
		                             : Controller
		                             ? Controller->GetPawn()
		                             : nullptr;
	if (Controller && RestoredViewTarget && Controller->GetViewTarget() != RestoredViewTarget)
	{
		return;
	}
	if (Controller && Controller->PlayerCameraManager && PartyPresentationExitBlendCompleteHandle.IsValid())
	{
		Controller->PlayerCameraManager->OnBlendComplete().Remove(PartyPresentationExitBlendCompleteHandle);
		PartyPresentationExitBlendCompleteHandle.Reset();
	}

	UnloadPartyPresentationLevel();
}

void UAetherPartySetupComponent::UnloadPartyPresentationLevel()
{
	ClearPartyPresentationLoadTimeout();
	if (PartyPresentationStreamingLevel)
	{
		PartyPresentationStreamingLevel->OnLevelShown.RemoveDynamic(
			this, &UAetherPartySetupComponent::HandlePartyPresentationLevelShown);
		PartyPresentationStreamingLevel->SetShouldBeVisible(false);
		PartyPresentationStreamingLevel->SetShouldBeLoaded(false);
		PartyPresentationStreamingLevel->SetIsRequestingUnloadAndRemoval(true);
	}
	else if (IsValid(PartyPresentationActor))
	{
		PartyPresentationActor->Destroy();
	}

	PartyPresentationActor = nullptr;
	PartyPresentationStreamingLevel = nullptr;
	PreviousViewTarget = nullptr;
	ReleasePartyPresentationStreamingSource();
	++PartyPresentationRequestId;
	PresentationState = EAetherPartyPresentationState::Idle;
}

void UAetherPartySetupComponent::RemovePartyPresentationCameraBlendDelegates()
{
	AAetherPlayerController* Controller = GetAetherController();
	if (Controller && Controller->PlayerCameraManager)
	{
		if (PartyPresentationBlendCompleteHandle.IsValid())
		{
			Controller->PlayerCameraManager->OnBlendComplete().Remove(PartyPresentationBlendCompleteHandle);
		}
		if (PartyPresentationExitBlendCompleteHandle.IsValid())
		{
			Controller->PlayerCameraManager->OnBlendComplete().Remove(PartyPresentationExitBlendCompleteHandle);
		}
	}
	PartyPresentationBlendCompleteHandle.Reset();
	PartyPresentationExitBlendCompleteHandle.Reset();
}

void UAetherPartySetupComponent::ClearPartyPresentationLoadTimeout()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PartyPresentationLoadTimeoutHandle);
	}
}

void UAetherPartySetupComponent::HandleGameMenuDisplayedWidgetChanged(
	UCommonActivatableWidget* DisplayedWidget)
{
	if (!ActivePartySelectWidget || !PrimaryGameLayout)
	{
		return;
	}

	UCommonActivatableWidgetStack* GameMenuStack =
		PrimaryGameLayout->GetLayerStack(AetherGameplayTags::UI_Layer_GameMenu);
	if (ActivePartySetupWidget
		&& (!GameMenuStack || !GameMenuStack->GetWidgetList().Contains(ActivePartySetupWidget)))
	{
		ActivePartySetupWidget = nullptr;
		if (IsValid(PartyPresentationActor))
		{
			PartyPresentationActor->ShowPartyOverview();
		}

		if (!bPartySelectionPending)
		{
			ResetPartySelection(true);
		}
	}

	if (!GameMenuStack || !GameMenuStack->GetWidgetList().Contains(ActivePartySelectWidget))
	{
		FinishPartySetupFlow();
		return;
	}

	if (DisplayedWidget == ActivePartySelectWidget)
	{
		BeginPartyPresentation();
	}
}

void UAetherPartySetupComponent::HandlePartyPresentationBlendComplete(const uint32 RequestId)
{
	AAetherPlayerController* Controller = GetAetherController();
	if (RequestId == PartyPresentationRequestId
		&& Controller
		&& ActivePartySelectWidget
		&& PresentationState == EAetherPartyPresentationState::Active
		&& IsValid(PartyPresentationActor)
		&& Controller->GetViewTarget() == PartyPresentationActor
		&& !Controller->IsPaused())
	{
		if (Controller->PlayerCameraManager && PartyPresentationBlendCompleteHandle.IsValid())
		{
			Controller->PlayerCameraManager->OnBlendComplete().Remove(PartyPresentationBlendCompleteHandle);
			PartyPresentationBlendCompleteHandle.Reset();
		}
		PauseGameIfNeeded();
	}
}

void UAetherPartySetupComponent::FinishPartySetupFlow()
{
	ActivePartySelectWidget = nullptr;
	ActivePartySetupWidget = nullptr;
	ResetPartySelection();
	UnpauseGameIfNeeded();
	EndPartyPresentation();
}

void UAetherPartySetupComponent::ResetPartySelection(const bool bForce)
{
	if (IsValid(PartyPresentationActor))
	{
		PartyPresentationActor->ShowPartyOverview();
	}

	if (bForce || !bPartySelectionPending)
	{
		SelectedPartySlotIndex = INDEX_NONE;
		PartySelectionPreviewCharacterId = FPrimaryAssetId();
	}
}

void UAetherPartySetupComponent::HandlePartySelectionCompleted(const bool bSucceeded)
{
	bPartySelectionPending = false;
	OnPartySelectionCompleted.Broadcast(bSucceeded);

	if (bSucceeded && ActivePartySetupWidget)
	{
		ActivePartySetupWidget->DeactivateWidget();
		return;
	}

	if (!ActivePartySetupWidget)
	{
		ResetPartySelection(true);
	}
}

void UAetherPartySetupComponent::HandlePresentedPartyChanged()
{
	UAetherPartyComponent* PartyComponent = GetPartyComponent();
	if (IsValid(PartyPresentationActor) && PartyComponent)
	{
		PartyPresentationActor->SetPartyCharacters(PartyComponent->GetPartyCharacterIds());
	}
}

void UAetherPartySetupComponent::PauseGameIfNeeded()
{
	AAetherPlayerController* Controller = GetAetherController();
	if (Controller && !Controller->IsPaused())
	{
		bPartySetupPausedGame = Controller->SetPause(true);
	}
}

void UAetherPartySetupComponent::UnpauseGameIfNeeded()
{
	if (bPartySetupPausedGame)
	{
		if (AAetherPlayerController* Controller = GetAetherController())
		{
			Controller->SetPause(false);
		}
		bPartySetupPausedGame = false;
	}
}

AAetherPlayerController* UAetherPartySetupComponent::GetAetherController() const
{
	return Cast<AAetherPlayerController>(GetOwner());
}

UAetherPartyComponent* UAetherPartySetupComponent::GetPartyComponent() const
{
	AAetherPlayerController* Controller = GetAetherController();
	AAetherPlayerState* AetherPlayerState = Controller
		                                        ? Controller->GetPlayerState<AAetherPlayerState>()
		                                        : nullptr;
	return AetherPlayerState ? AetherPlayerState->GetPartyComponent() : nullptr;
}
