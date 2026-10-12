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
	// Accepts a Gameplay+Display-loaded definition once; false means invalid input or repeated setup.
	bool InitializeCharacterRuntime(UAetherCharacterDefinition* Definition, AActor* AvatarActor);
	// Updates the GAS relationship only. The caller cancels abilities before clearing the avatar.
	void SetAvatarActor(AActor* AvatarActor);
	UAetherAbilitySystemComponent* GetAetherAbilitySystemComponent() const { return AetherASC; }

	
	USkeletalMesh* GetSkeletalMesh() const;
	TSubclassOf<UAnimInstance> GetAnimInstanceClass() const;

private:
	UPROPERTY()
	TObjectPtr<UAetherCharacterDefinition> CharacterDefinition;
	UPROPERTY()
	TObjectPtr<UAetherAbilitySystemComponent> AetherASC;
	UPROPERTY()
	TObjectPtr<UAetherBaseAttributeSet> BaseAttributeSet;
	
	FAetherAbilitySet_GrantedHandles GrantedAbilitySetHandles;
	
	
	
};
