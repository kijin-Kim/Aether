// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Aether/AbilitySystem/AetherAbilitySet.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/Weapon/WeaponHolder.h"
#include "GameFramework/Character.h"
#include "AetherCharacterBase.generated.h"

class UAetherCharacterData;

UCLASS(Abstract)
class AETHER_API AAetherCharacterBase : public ACharacter, public IAbilitySystemInterface, public IWeaponHolder
{
	GENERATED_BODY()

public:
	AAetherCharacterBase();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	UFUNCTION(BlueprintCallable, Category = "Aether|Character")
	void InitializeFromCharacterData(FName NewCharacterId);
	void SetOnField(bool bSetOnField);

	virtual UStaticMeshComponent* GetWeaponMesh() const override { return WeaponMeshComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AetherASC; }
	UAetherAbilitySystemComponent* GetAetherAbilitySystemComponent() const { return AetherASC; }

protected:
	FName CharacterId;

	UPROPERTY()
	TObjectPtr<UAetherAbilitySystemComponent> AetherASC;

	bool bOnField = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WeaponMeshComponent;

	UPROPERTY(EditAnywhere)
	FName WeaponSocketName = NAME_None;

	FAetherAbilitySet_GrantedHandles GrantedAbilitySetHandles;
};