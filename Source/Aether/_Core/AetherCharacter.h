// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AetherAbilitySet.h"
#include "AetherAbilitySystemComponent.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "AetherCharacter.generated.h"

class UAetherAttributeSet;
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
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_PlayerState() override;
	
	void SetOnField(bool bSetOnField);
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AetherASC; }
	UAetherAbilitySystemComponent* GetAetherAbilitySystemComponent() const { return AetherASC; }

private:
	void AbilityInputPressed(FGameplayTag InputTag);
	void AbilityInputReleased(FGameplayTag InputTag);
	
	void Move(const FInputActionValue& InputActionValue);
	void Look(const FInputActionValue& InputActionValue);
	
	UFUNCTION()
	void OnRep_OnField();
	
private:
	UPROPERTY()
	TObjectPtr<UAetherAbilitySystemComponent> AetherASC;
	UPROPERTY()
	TObjectPtr<UAetherAttributeSet> AetherAttributeSet;
	
	UPROPERTY(ReplicatedUsing=OnRep_OnField)
	bool bOnField = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;
	
	
	UPROPERTY(EditDefaultsOnly, Category = "Aether|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<const UAetherInputConfig> InputConfig;
	UPROPERTY(EditDefaultsOnly, Category = "Aether|Abilities", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<const UAetherAbilitySet> AbilitySet;
	FAetherAbilitySet_GrantedHandles GrantedAbilitySetHandles;
	
	
	
};
