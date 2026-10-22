#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherHealingFieldActor.generated.h"

class UAbilitySystemComponent;
class UAetherPartyComponent;
class USceneComponent;
class UStaticMeshComponent;
class UMaterialInterface;
struct FOnAttributeChangeData;

// Owns the field lifetime, periodic healing and overflow distribution.
UCLASS(Blueprintable)
class AETHER_API AAetherHealingFieldActor : public AActor
{
	GENERATED_BODY()

public:
	AAetherHealingFieldActor();
	void StartField(UAbilitySystemComponent* InCaster, UAetherPartyComponent* InParty, float InHealingPerTick);
	UAbilitySystemComponent* GetCasterASC() const { return CasterASC.Get(); }
	UAetherPartyComponent* GetSourceParty() const { return SourceParty.Get(); }
	int32 GetExecutedHealingTicks() const { return ExecutedHealingTicks; }
	int32 GetTotalHealingTicks() const { return HealingTickCount; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	UPROPERTY(VisibleAnywhere, Category = "Field")
	TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Field")
	TObjectPtr<UStaticMeshComponent> FieldVisual;
	UPROPERTY(EditDefaultsOnly, Category = "Field", meta = (ClampMin = "1"))
	float Radius = 500.f;
	UPROPERTY(EditDefaultsOnly, Category = "Field")
	TObjectPtr<UMaterialInterface> FieldMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Field", meta = (ClampMin = "0"))
	float HeightTolerance = 100.f;

private:
	void ExecuteHealingTick();
	void HealCurrentTarget();
	void ApplyHealing(UAbilitySystemComponent* Target, float RequestedAmount);
	void OnCasterHealthChanged(const FOnAttributeChangeData& Change);

	static constexpr int32 HealingTickCount = 10;
	static constexpr float HealingTickInterval = 1.f;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CasterASC;
	UPROPERTY()
	TWeakObjectPtr<UAetherPartyComponent> SourceParty;
	FDelegateHandle CasterHealthChangedHandle;
	FTimerHandle HealingTimer;
	float HealingPerTick = 0.f;
	int32 ExecutedHealingTicks = 0;
	bool bHasSourceParty = false;
	bool bStarted = false;
};
