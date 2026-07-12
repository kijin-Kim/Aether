// AetherPlayerState.h
#pragma once

#include "CoreMinimal.h"
#include "Aether/PartySystem/AetherPartyAbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"

#include "AetherPlayerState.generated.h"

class UAetherPartyAttributeSet;
class UAetherAbilitySystemComponent;
class UAetherPartyComponent;
class AAetherCharacter;
class APlayerController;
/**
 *
 *
 */
UCLASS()
class AETHER_API AAetherPlayerState : public APlayerState, public IAetherPartyAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAetherPlayerState();
	
	void ProcessInputs();
	virtual void DisplayDebug(class UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	
	
	UAetherAbilitySystemComponent* GetPartyASC() const { return PartyASC; }
	UAetherPartyComponent* GetPartyComponent() const { return PartyComponent; }
	
	virtual UAetherAbilitySystemComponent* GetPartyAbilitySystemComponent() const override { return PartyASC; }
	virtual UAetherAbilitySystemComponent* GetActiveAbilitySystemComponent() const override;
	
	
private:
	UPROPERTY()
	TObjectPtr<UAetherAbilitySystemComponent> PartyASC;
	UPROPERTY()
	TObjectPtr<UAetherPartyAttributeSet> PartyAttributeSet;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherPartyComponent> PartyComponent;
	
};
