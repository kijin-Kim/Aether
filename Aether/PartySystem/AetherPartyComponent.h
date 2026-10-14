// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherPartyComponent.generated.h"


class UAetherCharacterDefinition;
class UAetherAbilitySet;
class UAetherAbilitySystemComponent;
class AAetherFieldCharacterPawn;
class AAetherPartyMemberStateActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAetherPartyChanged);
DECLARE_DYNAMIC_DELEGATE(FOnAetherPartyChangedCallback);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAetherPartyMemberReplaceCompleted, bool, bSucceeded);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AETHER_API UAetherPartyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxPartyMembers = 4;

	UAetherPartyComponent();
	void InitializeParty(const TArray<FPrimaryAssetId>& CharacterIds);
	// The shared field pawn is assigned once for the lifetime of this party.
	void InitializeFieldPawn(AAetherFieldCharacterPawn* Pawn);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintAssignable, Category = "Aether|Party")
	FOnAetherPartyChanged OnPartyChanged;

	UFUNCTION(BlueprintCallable, Category = "Aether|Party", meta = (DisplayName = "Register and Call Party Changed"))
	void RegisterAndCallPartyChanged(UPARAM(DisplayName = "Event") FOnAetherPartyChangedCallback Callback);

	bool RequestSwitchPartySlot(int32 SlotIndex);
	// Returns request acceptance; OnCompleted reports the replacement result and may run before return.
	bool RequestReplacePartyMember(int32 SlotIndex, const FPrimaryAssetId& NewCharacterId,
	                               FOnAetherPartyMemberReplaceCompleted OnCompleted);


	AAetherFieldCharacterPawn* GetFieldCharacterPawn() const { return FieldCharacterPawn; }
	UFUNCTION(BlueprintPure, Category = "Aether|Party")
	TArray<FPrimaryAssetId> GetPartyCharacterIds() const { return PartyCharacterIds; }
	UFUNCTION(BlueprintPure, Category = "Aether|Party")
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }
	UFUNCTION(BlueprintPure, Category = "Aether|Party")
	// Composition can be configured before the shared pawn is ready.
	bool IsPartyInitialized() const { return bPartyConfigured; }
	UFUNCTION(BlueprintPure, Category = "Aether|Party")
	bool IsCharacterInParty(FPrimaryAssetId CharacterId) const { return PartyCharacterIds.Contains(CharacterId); }
	UAetherAbilitySystemComponent* GetPartyAbilitySystemComponent() const;
	UAetherAbilitySystemComponent* GetActiveAbilitySystemComponent() const;

private:
	friend class FAetherPartyInitializationTest;
	friend class FAetherPartySwitchPreparationTest;

	void TryInitializePartyRuntime();
	bool SwapPartySlots(int32 SlotIndex, int32 ExistingSlotIndex);
	bool ReplacePartyMember(int32 SlotIndex, UAetherCharacterDefinition* Definition);
	bool ApplyPartyMemberToFieldPawn(int32 SlotIndex);

	UPROPERTY(EditDefaultsOnly, Category = "Aether|Party")
	TObjectPtr<UAetherAbilitySet> PartyAbilitySet;
	UPROPERTY()
	TObjectPtr<AAetherFieldCharacterPawn> FieldCharacterPawn;
	int32 ActiveSlotIndex = INDEX_NONE;


	UPROPERTY(BlueprintReadOnly, Category = "Aether|Party", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AAetherPartyMemberStateActor>> PartyMemberStateActors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|Party", meta = (AllowPrivateAccess = "true"))
	TArray<FPrimaryAssetId> PartyCharacterIds;

	bool bPartyConfigured = false;

};
