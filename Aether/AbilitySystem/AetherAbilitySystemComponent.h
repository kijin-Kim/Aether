// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AetherGameplayAbility.h"
#include "AetherAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AETHER_API UAetherAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UAetherAbilitySystemComponent();
	void ProcessInputs();
	void ClearAbilityInputs();
	void ClearAllAbilityInputs();
	bool HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy SwapPolicy) const;
	void CancelActiveAbilitiesWithSwapPolicy(EAetherAbilitySwapPolicy SwapPolicy);
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	void AbilityInputPressed(const FGameplayTag& InputTag);
	void AbilityInputReleased(const FGameplayTag& InputTag);
	
private:
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
};
