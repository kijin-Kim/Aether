// AetherPlayerState.h
#pragma once

#include "CoreMinimal.h"
#include "Aether/AbilitySystem/AetherCharacterDefinition.h"
#include "Aether/PartySystem/AetherPartyComponent.h"
#include "GameFramework/PlayerState.h"

#include "AetherPlayerState.generated.h"

class UAetherAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAetherPlayerDataInitialized);
DECLARE_DYNAMIC_DELEGATE(FOnAetherPlayerDataInitializedCallback);

/**
 *
 *
 */




UCLASS(BlueprintType)
class AETHER_API UAetherInitialPlayerData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAetherOwnedCharacterData> InitialRoster;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FPrimaryAssetId> InitialParty;
};

UCLASS()
class AETHER_API AAetherPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AAetherPlayerState();
	virtual void BeginPlay() override;
	void ProcessInputs();
	virtual void DisplayDebug(class UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL,
	                          float& YPos) override;

	UPROPERTY(BlueprintAssignable, Category = "Aether|Player Data")
	FOnAetherPlayerDataInitialized OnPlayerDataInitialized;

	UFUNCTION(BlueprintCallable, Category = "Aether|Player Data",
		meta = (DisplayName = "Call or Register Player Data Initialized"))
	void CallOrRegisterPlayerDataInitialized(
		UPARAM(DisplayName = "Event") FOnAetherPlayerDataInitializedCallback Callback);

	UAetherAbilitySystemComponent* GetPartyASC() const { return PartyASC; }
	UFUNCTION(BlueprintPure, Category = "Aether|Party")
	UAetherPartyComponent* GetPartyComponent() const { return PartyComponent; }
	UFUNCTION(BlueprintPure, Category = "Aether|Roster")
	TArray<FAetherOwnedCharacterData> GetOwnedRoster() const { return OwnedRoster; }
	UFUNCTION(BlueprintCallable, Category = "Aether|Party")
	bool RequestReplacePartyMember(int32 SlotIndex, FPrimaryAssetId NewCharacterId, UPARAM(DisplayName = "Completed") FOnAetherPartyMemberReplaceCompleted OnCompleted);

	UAetherAbilitySystemComponent* GetActiveAbilitySystemComponent() const;

private:
	UPROPERTY()
	TObjectPtr<UAetherAbilitySystemComponent> PartyASC;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherPartyComponent> PartyComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|Roster", meta = (AllowPrivateAccess = "true"))
	TArray<FAetherOwnedCharacterData> OwnedRoster;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|Player Data", meta = (AllowPrivateAccess = "true"))
	bool bPlayerDataInitialized = false;

	UPROPERTY(EditDefaultsOnly, Category = "Initial Data")
	TObjectPtr<UAetherInitialPlayerData> InitialPlayerData;

};
