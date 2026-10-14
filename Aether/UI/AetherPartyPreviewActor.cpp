#include "AetherPartyPreviewActor.h"

#include "Aether/Player/AetherPlayerState.h"
#include "Kismet/GameplayStatics.h"
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
	SceneCapture->TextureTarget = nullptr;
	RenderTarget = nullptr;

	Super::EndPlay(EndPlayReason);
}

void AAetherPartyPreviewActor::SetPreviewCharacter(FPrimaryAssetId CharacterId)
{
	PreviewMesh->SetSkeletalMesh(nullptr);
	PreviewMesh->SetAnimInstanceClass(nullptr);

	const AAetherPlayerState* PS = Cast<AAetherPlayerState>(UGameplayStatics::GetPlayerState(this, 0));
	UAetherCharacterDefinition* Definition = PS ? PS->GetCharacterDefinition(CharacterId) : nullptr;
	if (!Definition || !Definition->DisplayMesh)
	{
		OnPreviewChanged.Broadcast(false);
		return;
	}
	PreviewMesh->SetSkeletalMesh(Definition->DisplayMesh);
	PreviewMesh->SetAnimInstanceClass(Definition->DisplayAnimInstanceClass);
	SceneCapture->CaptureScene();
	OnPreviewChanged.Broadcast(true);
}

void AAetherPartyPreviewActor::ClearPreview()
{
	PreviewMesh->SetSkeletalMesh(nullptr);
	PreviewMesh->SetAnimInstanceClass(nullptr);
}
