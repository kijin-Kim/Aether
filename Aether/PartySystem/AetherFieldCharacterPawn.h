// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AetherFieldCharacterPawn.generated.h"

class UAetherAbilitySystemComponent;
class UAetherGameplayAbility_NormalAttack;
struct FAetherTraceKey;
struct FAetherMeleeTraceSettings;
struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;
class UWorldPartitionStreamingSourceComponent;

UCLASS()
class AETHER_API AAetherFieldCharacterPawn : public ACharacter
{
	GENERATED_BODY()

public:
	AAetherFieldCharacterPawn();
	bool ApplyCharacterAppearance(USkeletalMesh* SkeletalMesh, TSubclassOf<UAnimInstance> AnimInstanceClass);
	void SetFieldStreamingSourceEnabled(bool bEnabled);
	uint64 RegisterAttackReceiver(UAetherGameplayAbility_NormalAttack* Receiver);
	void UnregisterAttackReceiver(UAetherGameplayAbility_NormalAttack* Receiver, uint64 Registration);
	void BeginAttackTrace(const FAetherTraceKey& Key, const FAetherMeleeTraceSettings& Settings);
	void UpdateAttackTrace(const FAetherTraceKey& Key);
	void EndAttackTrace(const FAetherTraceKey& Key);

private:
	TWeakObjectPtr<UAetherGameplayAbility_NormalAttack> AttackReceiver;
	uint64 AttackRegistration = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWorldPartitionStreamingSourceComponent> FieldStreamingSourceComponent;
};
