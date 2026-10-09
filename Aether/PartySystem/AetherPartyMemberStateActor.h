// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Aether/AbilitySystem/AetherAbilitySet.h"
#include "GameFramework/Actor.h"
#include "AetherPartyMemberStateActor.generated.h"

class UAetherCharacterDefinition;
class UAetherBaseAttributeSet;
class UAetherAbilitySystemComponent;

UCLASS()
class AETHER_API AAetherPartyMemberStateActor : public AActor
{
	GENERATED_BODY()

public:
	AAetherPartyMemberStateActor();
	void InitializePartyMemberStateActor(FPrimaryAssetId Id, AActor* AvatarActor);
	UAetherAbilitySystemComponent* GetAetherAbilitySystemComponent() const { return AetherASC; }

	
	USkeletalMesh* GetSkeletalMesh() const;
	TSubclassOf<UAnimInstance> GetAnimInstanceClass() const;

private:
	UAetherCharacterDefinition* GetCharacterData() const;
	
	
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FPrimaryAssetId CharacterDefinitionId;
	UPROPERTY()
	TObjectPtr<UAetherAbilitySystemComponent> AetherASC;
	UPROPERTY()
	TObjectPtr<UAetherBaseAttributeSet> BaseAttributeSet;
	
	FAetherAbilitySet_GrantedHandles GrantedAbilitySetHandles;
	
	
	
};
