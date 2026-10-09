#include "AetherPartyPresentationActor.h"

#include "Aether/AetherCharacterAssetSubsystem.h"
#include "Aether/AbilitySystem/AetherCharacterDefinition.h"
#include "Aether/PartySystem/AetherPartyComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"

AAetherPartyPresentationActor::AAetherPartyPresentationActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorEnableCollision(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PresentationCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PresentationCamera"));
	PresentationCamera->SetupAttachment(SceneRoot);
	PresentationCamera->SetRelativeLocation(FVector(650.f, 0.f, 110.f));
	PresentationCamera->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	PresentationCamera->SetFieldOfView(42.f);

	KeyLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(SceneRoot);
	KeyLight->SetRelativeLocation(FVector(250.f, -250.f, 300.f));
	KeyLight->SetIntensity(8000.f);
	KeyLight->SetAttenuationRadius(1200.f);

	FillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(SceneRoot);
	FillLight->SetRelativeLocation(FVector(100.f, 300.f, 180.f));
	FillLight->SetIntensity(3500.f);
	FillLight->SetAttenuationRadius(1000.f);

	OverviewCharacterLocations = {
		FVector(0.f, 240.f, 0.f),
		FVector(0.f, 80.f, 0.f),
		FVector(0.f, -80.f, 0.f),
		FVector(0.f, -240.f, 0.f)
	};

	for (int32 SlotIndex = 0; SlotIndex < UAetherPartyComponent::MaxPartyMembers; ++SlotIndex)
	{
		const FName AnchorName(*FString::Printf(TEXT("CharacterAnchor%d"), SlotIndex));
		USceneComponent* CharacterAnchor = CreateDefaultSubobject<USceneComponent>(AnchorName);
		CharacterAnchor->SetupAttachment(SceneRoot);
		CharacterAnchor->SetRelativeLocation(OverviewCharacterLocations[SlotIndex]);
		CharacterAnchors.Add(CharacterAnchor);

		const FName MeshName(*FString::Printf(TEXT("CharacterMesh%d"), SlotIndex));
		USkeletalMeshComponent* CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(MeshName);
		CharacterMesh->SetupAttachment(CharacterAnchor);
		CharacterMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CharacterMesh->PrimaryComponentTick.bTickEvenWhenPaused = true;
		CharacterMesh->SetVisibility(false);
		CharacterMeshes.Add(CharacterMesh);
	}

	PreviewCharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewCharacterMesh"));
	PreviewCharacterMesh->SetupAttachment(SceneRoot);
	PreviewCharacterMesh->SetRelativeLocation(FocusedCharacterLocation);
	PreviewCharacterMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	PreviewCharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewCharacterMesh->PrimaryComponentTick.bTickEvenWhenPaused = true;
	PreviewCharacterMesh->SetVisibility(false);
}

void AAetherPartyPresentationActor::SetPartyCharacters(const TArray<FPrimaryAssetId>& CharacterIds)
{
	PartyCharacterIds.Reset();
	const int32 CharacterCount = FMath::Min(CharacterIds.Num(), UAetherPartyComponent::MaxPartyMembers);
	PartyCharacterIds.Append(CharacterIds.GetData(), CharacterCount);

	for (USkeletalMeshComponent* CharacterMesh : CharacterMeshes)
	{
		if (CharacterMesh)
		{
			CharacterMesh->SetSkeletalMesh(nullptr);
			CharacterMesh->SetAnimInstanceClass(nullptr);
			CharacterMesh->SetVisibility(false);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	UAetherCharacterAssetSubsystem* CharacterAssetSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UAetherCharacterAssetSubsystem>() : nullptr;
	if (!CharacterAssetSubsystem)
	{
		return;
	}

	for (const FPrimaryAssetId& CharacterId : PartyCharacterIds)
	{
		if (!CharacterId.IsValid())
		{
			continue;
		}

		FOnCharacterDefinitionLoaded OnLoaded;
		OnLoaded.BindDynamic(this, &AAetherPartyPresentationActor::HandleCharacterDisplayLoaded);
		CharacterAssetSubsystem->LoadCharacterDisplay(CharacterId, MoveTemp(OnLoaded));
	}

	UpdateCharacterPresentation();
}

void AAetherPartyPresentationActor::SetFocusedPartySlot(const int32 SlotIndex)
{
	bShowingCharacterPreview = false;
	PreviewCharacterId = FPrimaryAssetId();
	if (PreviewCharacterMesh)
	{
		PreviewCharacterMesh->SetSkeletalMesh(nullptr);
		PreviewCharacterMesh->SetAnimInstanceClass(nullptr);
	}

	FocusedSlotIndex = PartyCharacterIds.IsValidIndex(SlotIndex) ? SlotIndex : INDEX_NONE;
	UpdateCharacterPresentation();
}

void AAetherPartyPresentationActor::ShowPartyOverview()
{
	bShowingCharacterPreview = false;
	PreviewCharacterId = FPrimaryAssetId();
	FocusedSlotIndex = INDEX_NONE;

	if (PreviewCharacterMesh)
	{
		PreviewCharacterMesh->SetSkeletalMesh(nullptr);
		PreviewCharacterMesh->SetAnimInstanceClass(nullptr);
	}

	UpdateCharacterPresentation();
}

void AAetherPartyPresentationActor::ShowCharacterPreview(const FPrimaryAssetId CharacterId)
{
	if (!CharacterId.IsValid())
	{
		ShowPartyOverview();
		return;
	}

	if (bShowingCharacterPreview
		&& PreviewCharacterId == CharacterId
		&& PreviewCharacterMesh
		&& PreviewCharacterMesh->GetSkeletalMeshAsset())
	{
		UpdateCharacterPresentation();
		return;
	}

	bShowingCharacterPreview = true;
	PreviewCharacterId = CharacterId;
	FocusedSlotIndex = INDEX_NONE;

	if (PreviewCharacterMesh)
	{
		PreviewCharacterMesh->SetSkeletalMesh(nullptr);
		PreviewCharacterMesh->SetAnimInstanceClass(nullptr);
	}
	UpdateCharacterPresentation();

	UGameInstance* GameInstance = GetGameInstance();
	UAetherCharacterAssetSubsystem* CharacterAssetSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UAetherCharacterAssetSubsystem>() : nullptr;
	if (!CharacterAssetSubsystem)
	{
		return;
	}

	FOnCharacterDefinitionLoaded OnLoaded;
	OnLoaded.BindDynamic(this, &AAetherPartyPresentationActor::HandlePreviewCharacterDisplayLoaded);
	CharacterAssetSubsystem->LoadCharacterDisplay(CharacterId, MoveTemp(OnLoaded));
}

void AAetherPartyPresentationActor::HandleCharacterDisplayLoaded(
	UAetherCharacterDefinition* LoadedDefinition)
{
	if (!LoadedDefinition)
	{
		return;
	}

	const int32 SlotIndex = PartyCharacterIds.IndexOfByKey(LoadedDefinition->GetPrimaryAssetId());
	if (!CharacterMeshes.IsValidIndex(SlotIndex) || !CharacterMeshes[SlotIndex])
	{
		return;
	}

	USkeletalMesh* LoadedMesh = LoadedDefinition->DisplayMesh.Get();
	if (!LoadedMesh)
	{
		return;
	}

	CharacterMeshes[SlotIndex]->SetSkeletalMesh(LoadedMesh);
	CharacterMeshes[SlotIndex]->SetAnimInstanceClass(LoadedDefinition->DisplayAnimInstanceClass.Get());
	UpdateCharacterPresentation();
}

void AAetherPartyPresentationActor::HandlePreviewCharacterDisplayLoaded(
	UAetherCharacterDefinition* LoadedDefinition)
{
	if (!bShowingCharacterPreview
		|| !LoadedDefinition
		|| LoadedDefinition->GetPrimaryAssetId() != PreviewCharacterId
		|| !PreviewCharacterMesh)
	{
		return;
	}

	USkeletalMesh* LoadedMesh = LoadedDefinition->DisplayMesh.Get();
	if (!LoadedMesh)
	{
		return;
	}

	PreviewCharacterMesh->SetSkeletalMesh(LoadedMesh);
	PreviewCharacterMesh->SetAnimInstanceClass(LoadedDefinition->DisplayAnimInstanceClass.Get());
	UpdateCharacterPresentation();
}

void AAetherPartyPresentationActor::UpdateCharacterPresentation()
{
	for (int32 SlotIndex = 0; SlotIndex < CharacterMeshes.Num(); ++SlotIndex)
	{
		USkeletalMeshComponent* CharacterMesh = CharacterMeshes[SlotIndex];
		USceneComponent* CharacterAnchor =
			CharacterAnchors.IsValidIndex(SlotIndex) ? CharacterAnchors[SlotIndex] : nullptr;
		if (!CharacterMesh || !CharacterAnchor)
		{
			continue;
		}

		const bool bHasLoadedCharacter = CharacterMesh->GetSkeletalMeshAsset() != nullptr;
		const bool bShouldShow =
			!bShowingCharacterPreview
			&& bHasLoadedCharacter
			&& (FocusedSlotIndex == INDEX_NONE || FocusedSlotIndex == SlotIndex);
		CharacterMesh->SetVisibility(bShouldShow);

		const FVector TargetLocation =
			FocusedSlotIndex == SlotIndex ? FocusedCharacterLocation : OverviewCharacterLocations[SlotIndex];
		CharacterAnchor->SetRelativeLocation(TargetLocation);
	}

	if (PreviewCharacterMesh)
	{
		PreviewCharacterMesh->SetRelativeLocation(FocusedCharacterLocation);
		PreviewCharacterMesh->SetVisibility(
			bShowingCharacterPreview && PreviewCharacterMesh->GetSkeletalMeshAsset() != nullptr);
	}
}
