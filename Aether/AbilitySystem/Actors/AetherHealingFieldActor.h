#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherHealingFieldActor.generated.h"

class UAbilitySystemComponent;
class UAetherPartyComponent;
class UStaticMeshComponent;
class UMaterialInterface;
struct FOnAttributeChangeData;

// Independent field lifetime and periodic healing. Includes overflow distribution; cooldown is a separate stage.
UCLASS(Blueprintable)
class AETHER_API AAetherHealingFieldActor : public AActor
{
	GENERATED_BODY()
public:
	AAetherHealingFieldActor();
	void StartField(UAbilitySystemComponent* InCaster, UAetherPartyComponent* InParty, float InHealingPerTick);
	UAbilitySystemComponent* GetCasterASC() const { return CasterASC.Get(); }
	UAetherPartyComponent* GetSourceParty() const { return SourceParty.Get(); }
	float GetDuration() const { return Duration; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Field")
	TObjectPtr<UStaticMeshComponent> FieldVisual;
	UPROPERTY(EditDefaultsOnly, Category="Field", meta=(ClampMin="1"))
	float Radius = 500.f;
	UPROPERTY(EditDefaultsOnly, Category="Field", meta=(ClampMin="0.01"))
	float Duration = 10.f;
	UPROPERTY(EditDefaultsOnly, Category="Field")
	TObjectPtr<UMaterialInterface> FieldMaterial;

	UPROPERTY(EditDefaultsOnly, Category="Field", meta=(ClampMin="0"))
	float HeightTolerance = 100.f;

private:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CasterASC;
	UPROPERTY()
	TWeakObjectPtr<UAetherPartyComponent> SourceParty;
	FDelegateHandle CasterHealthChangedHandle;
	void OnCasterHealthChanged(const FOnAttributeChangeData& Change);
	FTimerHandle ExpirationTimer;
	FTimerHandle HealingTimer;
	float HealingPerTick = 0.f;
	int32 NextHealingSlot = 0;
	bool bHasSourceParty = false;
	void HealAndScheduleNext();
	void HealCurrentTarget();
	void ApplyHealing(UAbilitySystemComponent* Target, float RequestedAmount);
	bool bStarted = false;
	void Expire();
};
