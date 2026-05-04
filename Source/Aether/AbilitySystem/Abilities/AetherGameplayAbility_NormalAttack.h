// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Aether/AbilitySystem/AetherGameplayAbility.h"
#include "AetherGameplayAbility_NormalAttack.generated.h"


UENUM(BlueprintType)
enum class EElement : uint8
{
	None,
	Pyro,
	Hydro,
	Electro,
	Cryo,
	Anemo,
	Geo,
	Dendro
};

/**
 * 
 */
UCLASS()
class AETHER_API UAetherGameplayAbility_NormalAttack : public UAetherGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	void PlayCombo();

private:
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnWindowOpenedEvent(FGameplayEventData Payload);
	UFUNCTION()
	void OnWindowClosedEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnInputPressedEvent(float TimeWaited);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Aether|Animation")
	TArray<TObjectPtr<UAnimMontage>> GroundAttackAnimations;

	int8 ComboIndex = 0;

	bool bComboWindowOpened = false;
	bool bInputBuffered = false;


	UPROPERTY(EditDefaultsOnly, Category = "Aether")
	TSubclassOf<UGameplayEffect> ElementalDamageGE;
};
