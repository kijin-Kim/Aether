#pragma once

#include "CoreMinimal.h"
#include "AetherActivatableWidget.h"
#include "AetherPartySetupWidget.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class AETHER_API UAetherPartySetupWidget : public UAetherActivatableWidget
{
	GENERATED_BODY()

public:
	void InitializePartySlot(int32 InPartySlotIndex);

protected:
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Aether|Party Setup")
	int32 TargetPartySlotIndex = INDEX_NONE;
};
