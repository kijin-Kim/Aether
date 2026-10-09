#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherPartyPresentationActor.generated.h"

class UAetherCharacterDefinition;
class UCameraComponent;
class UPointLightComponent;
class USceneComponent;
class USkeletalMeshComponent;

UCLASS(BlueprintType, Blueprintable)
class AETHER_API AAetherPartyPresentationActor : public AActor
{
	GENERATED_BODY()

public:
	AAetherPartyPresentationActor();

	UFUNCTION(BlueprintCallable, Category = "Aether|Party Presentation")
	void SetPartyCharacters(const TArray<FPrimaryAssetId>& CharacterIds);

	UFUNCTION(BlueprintCallable, Category = "Aether|Party Presentation")
	void SetFocusedPartySlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Aether|Party Presentation")
	void ShowPartyOverview();

	UFUNCTION(BlueprintCallable, Category = "Aether|Party Presentation")
	void ShowCharacterPreview(FPrimaryAssetId CharacterId);

	UFUNCTION(BlueprintPure, Category = "Aether|Party Presentation")
	int32 GetFocusedPartySlot() const { return FocusedSlotIndex; }

private:
	UFUNCTION()
	void HandleCharacterDisplayLoaded(UAetherCharacterDefinition* LoadedDefinition);

	UFUNCTION()
	void HandlePreviewCharacterDisplayLoaded(UAetherCharacterDefinition* LoadedDefinition);

	void UpdateCharacterPresentation();

	UPROPERTY(VisibleAnywhere, Category = "Aether|Party Presentation")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Aether|Party Presentation")
	TObjectPtr<UCameraComponent> PresentationCamera;

	UPROPERTY(VisibleAnywhere, Category = "Aether|Party Presentation")
	TObjectPtr<UPointLightComponent> KeyLight;

	UPROPERTY(VisibleAnywhere, Category = "Aether|Party Presentation")
	TObjectPtr<UPointLightComponent> FillLight;

	UPROPERTY(VisibleAnywhere, Category = "Aether|Party Presentation")
	TArray<TObjectPtr<USceneComponent>> CharacterAnchors;

	UPROPERTY(VisibleAnywhere, Category = "Aether|Party Presentation")
	TArray<TObjectPtr<USkeletalMeshComponent>> CharacterMeshes;

	UPROPERTY(VisibleAnywhere, Category = "Aether|Party Presentation")
	TObjectPtr<USkeletalMeshComponent> PreviewCharacterMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Aether|Party Presentation")
	FVector FocusedCharacterLocation = FVector(0.f, -170.f, 0.f);

	TArray<FVector> OverviewCharacterLocations;
	TArray<FPrimaryAssetId> PartyCharacterIds;
	FPrimaryAssetId PreviewCharacterId;
	int32 FocusedSlotIndex = INDEX_NONE;
	bool bShowingCharacterPreview = false;
};
