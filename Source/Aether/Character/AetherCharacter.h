// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Aether/AbilitySystem/AetherAbilitySet.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "AetherCharacter.generated.h"

class UAetherCharacterData;
class UAetherHeroAttributeSet;
class UAetherBaseAttributeSet;
class UAetherAbilitySystemComponent;
struct FGameplayTag;
class UAetherAbilitySet;
class UAetherInputConfig;
class UGameplayAbility;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;


UCLASS()
class AETHER_API AAetherCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAetherCharacter();
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	void InitializeFromCharacterData(FName NewCharacterId);


	void SetOnField(bool bSetOnField);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AetherASC; }
	UAetherAbilitySystemComponent* GetAetherAbilitySystemComponent() const { return AetherASC; }

private:
	void AbilityInputPressed(FGameplayTag InputTag);
	void AbilityInputReleased(FGameplayTag InputTag);

	void Move(const FInputActionValue& InputActionValue);
	void Look(const FInputActionValue& InputActionValue);

private:
	FName CharacterId;

	UPROPERTY()
	TObjectPtr<UAetherAbilitySystemComponent> AetherASC;

	bool bOnField = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;


	FAetherAbilitySet_GrantedHandles GrantedAbilitySetHandles;
};
