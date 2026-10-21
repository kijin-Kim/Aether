#pragma once

#include "CoreMinimal.h"
#include "Aether/AbilitySystem/AetherGameplayAbility.h"
#include "AetherGameplayAbility_HealingField.generated.h"

class AAetherHealingFieldActor;

UCLASS()
class AETHER_API UAetherGameplayAbility_HealingField : public UAetherGameplayAbility
{
	GENERATED_BODY()
public:
	UAetherGameplayAbility_HealingField();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	AAetherHealingFieldActor* GetSpawnedField() const { return SpawnedField.Get(); }

protected:
	UPROPERTY(EditDefaultsOnly, Category="Healing Field")
	TSubclassOf<AAetherHealingFieldActor> FieldClass;

	UPROPERTY(EditDefaultsOnly, Category="Healing Field", meta=(ClampMin="0", ClampMax="100"))
	float HealingPercent = 5.f;

private:
	// Retained across EndAbility. The field executes independently of this ability.
	UPROPERTY()
	TWeakObjectPtr<AAetherHealingFieldActor> SpawnedField;
};
