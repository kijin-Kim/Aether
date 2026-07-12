#include "AetherPlayerState.h"

#include "DisplayDebugHelpers.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AttributeSet/AetherPartyAttributeSet.h"
#include "Aether/PartySystem/AetherPartyComponent.h"
#include "GameFramework/PlayerController.h"


AAetherPlayerState::AAetherPlayerState()
{
	PartyASC = CreateDefaultSubobject<UAetherAbilitySystemComponent>(TEXT("PartyASC"));
	PartyAttributeSet = CreateDefaultSubobject<UAetherPartyAttributeSet>(TEXT("PartyAttributeSet"));
	PartyComponent = CreateDefaultSubobject<UAetherPartyComponent>(TEXT("PartyComponent"));
}

void AAetherPlayerState::ProcessInputs()
{
	PartyASC->ProcessInputs();
	UAetherAbilitySystemComponent* ActiveASC = GetActiveAbilitySystemComponent();
	if (ActiveASC)
	{
		ActiveASC->ProcessInputs();
	}
}

void AAetherPlayerState::DisplayDebug(class UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL,
                                      float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	if (DebugDisplay.IsDisplayOn(TEXT("AbilitySystem")))
	{
		GetPartyAbilitySystemComponent()->DisplayDebug(Canvas, DebugDisplay, YL, YPos);
		GetActiveAbilitySystemComponent()->DisplayDebug(Canvas, DebugDisplay, YL, YPos);
	}
}


UAetherAbilitySystemComponent* AAetherPlayerState::GetActiveAbilitySystemComponent() const
{
	return PartyComponent->GetActiveAbilitySystemComponent();
}
