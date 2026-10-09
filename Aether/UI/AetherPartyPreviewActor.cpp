#include "AetherPartyPreviewActor.h"

#include "Aether/AetherCharacterAssetSubsystem.h"
#include "Aether/AbilitySystem/AetherCharacterDefinition.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"

AAetherPartyPreviewActor::AAetherPartyPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(SceneRoot);
	PreviewMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	PreviewMesh->SetVisibleInSceneCaptureOnly(true);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(SceneRoot);
	SceneCapture->SetRelativeLocation(FVector(300.f, 0.f, 90.f));
	SceneCapture->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	SceneCapture->FOVAngle = 30.f;
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	SceneCapture->bCaptureEveryFrame = true;
	SceneCapture->bCaptureOnMovement = true;
}

void AAetherPartyPreviewActor::BeginPlay()
{
	Super::BeginPlay();

	RenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("PartyPreviewRenderTarget"));
	RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	RenderTarget->ClearColor = FLinearColor::Transparent;
	RenderTarget->InitAutoFormat(RenderTargetSize, RenderTargetSize);
	RenderTarget->UpdateResourceImmediate(true);

	SceneCapture->TextureTarget = RenderTarget;
	SceneCapture->ShowOnlyActorComponents(this);
}

void AAetherPartyPreviewActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PendingCharacterId = FPrimaryAssetId();
	SceneCapture->TextureTarget = nullptr;
	RenderTarget = nullptr;

	Super::EndPlay(EndPlayReason);
}

void AAetherPartyPreviewActor::SetPreviewCharacter(FPrimaryAssetId CharacterId)
{
	PendingCharacterId = CharacterId;
	PreviewMesh->SetSkeletalMesh(nullptr);
	PreviewMesh->SetAnimInstanceClass(nullptr);

	UGameInstance* GameInstance = GetGameInstance();
	UAetherCharacterAssetSubsystem* CharacterAssetSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UAetherCharacterAssetSubsystem>() : nullptr;
	if (!CharacterId.IsValid() || !CharacterAssetSubsystem)
	{
		OnPreviewChanged.Broadcast(false);
		return;
	}

	FOnCharacterDefinitionLoaded OnLoaded;
	OnLoaded.BindDynamic(this, &AAetherPartyPreviewActor::HandleCharacterDisplayLoaded);
	CharacterAssetSubsystem->LoadCharacterDisplay(CharacterId, MoveTemp(OnLoaded));
}

void AAetherPartyPreviewActor::ClearPreview()
{
	PendingCharacterId = FPrimaryAssetId();
	PreviewMesh->SetSkeletalMesh(nullptr);
	PreviewMesh->SetAnimInstanceClass(nullptr);
}

void AAetherPartyPreviewActor::HandleCharacterDisplayLoaded(UAetherCharacterDefinition* LoadedDefinition)
{
	if (!LoadedDefinition || LoadedDefinition->GetPrimaryAssetId() != PendingCharacterId)
	{
		if (!LoadedDefinition)
		{
			OnPreviewChanged.Broadcast(false);
		}
		return;
	}

	USkeletalMesh* LoadedMesh = LoadedDefinition->DisplayMesh.Get();
	UClass* LoadedAnimClass = LoadedDefinition->DisplayAnimInstanceClass.Get();
	if (!LoadedMesh)
	{
		OnPreviewChanged.Broadcast(false);
		return;
	}

	PreviewMesh->SetSkeletalMesh(LoadedMesh);
	PreviewMesh->SetAnimInstanceClass(LoadedAnimClass);
	SceneCapture->CaptureScene();
	OnPreviewChanged.Broadcast(true);
}
