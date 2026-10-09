#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherPartyPreviewActor.generated.h"

class UAetherCharacterDefinition;
class USceneCaptureComponent2D;
class USceneComponent;
class USkeletalMeshComponent;
class UTextureRenderTarget2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAetherPartyPreviewChanged, bool, bSucceeded);

UCLASS(BlueprintType)
class AETHER_API AAetherPartyPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	AAetherPartyPreviewActor();

	UFUNCTION(BlueprintCallable, Category = "Aether|Party Preview")
	void SetPreviewCharacter(FPrimaryAssetId CharacterId);

	UFUNCTION(BlueprintCallable, Category = "Aether|Party Preview")
	void ClearPreview();

	UFUNCTION(BlueprintPure, Category = "Aether|Party Preview")
	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

	UPROPERTY(BlueprintAssignable, Category = "Aether|Party Preview")
	FOnAetherPartyPreviewChanged OnPreviewChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleCharacterDisplayLoaded(UAetherCharacterDefinition* LoadedDefinition);

	UPROPERTY(VisibleAnywhere, Category = "Aether|Party Preview")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Aether|Party Preview")
	TObjectPtr<USkeletalMeshComponent> PreviewMesh;

	UPROPERTY(VisibleAnywhere, Category = "Aether|Party Preview")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Aether|Party Preview", meta = (ClampMin = "256", ClampMax = "2048"))
	int32 RenderTargetSize = 1024;

	FPrimaryAssetId PendingCharacterId;
};
