#pragma once
#include "CoreMinimal.h"
#include "Aether/AbilitySystem/AetherGameplayAbility.h"
#include "Aether/Combat/AetherMeleeTrace.h"
#include "AetherGameplayAbility_NormalAttack.generated.h"
class AAetherFieldCharacterPawn;

UCLASS()
class AETHER_API UAetherGameplayAbility_NormalAttack : public UAetherGameplayAbility
{
	GENERATED_BODY()
  public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData *TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	void BeginTrace(uint64 Registration, const FAetherTraceKey &Key, const FAetherMeleeTraceSettings &Settings);
	void UpdateTrace(uint64 Registration, const FAetherTraceKey &Key);
	void EndTrace(uint64 Registration, const FAetherTraceKey &Key);

  protected:
	// Preserve Blueprint data; stage 1 only plays entry zero.
	UPROPERTY(EditDefaultsOnly, Category = "Aether|Animation") TArray<TObjectPtr<UAnimMontage>> GroundAttackAnimations;
	UPROPERTY(EditDefaultsOnly, Category = "Aether|Damage", meta = (ClampMin = "0")) float BaseDamage = 100.f;
	// Empty means physical damage without aura application.
	UPROPERTY(EditDefaultsOnly, Category = "Aether|Damage", meta = (Categories = "Element")) FGameplayTag DamageElement;
	UPROPERTY(EditDefaultsOnly, Category = "Aether|Damage", meta = (ClampMin = "0")) float AuraGauge = 0.f;

  private:
	friend struct FNormalAttackTestAccess;
	UFUNCTION() void OnMontageCompleted();
	UFUNCTION() void OnMontageInterrupted();
	bool Accepts(uint64 Registration, const FAetherTraceKey &Key) const;
	TWeakObjectPtr<AAetherFieldCharacterPawn> TracePawn;
	uint64 TraceRegistration = 0;
	int32 MontageInstanceId = INDEX_NONE;
	TMap<FAetherTraceKey, FAetherTraceSession> TraceSessions;
};
