#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "AetherGameplayEffect_HealingField.generated.h"

UCLASS()
class AETHER_API UAetherGameplayEffect_HealingField : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAetherGameplayEffect_HealingField();
	static const FName AmountName;
};
