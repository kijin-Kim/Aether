// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Aether/AbilitySystem/AetherAbilitySet.h"
#include "GameFramework/Actor.h"
#include "AetherPartyMemberStateActor.generated.h"

class UAetherCharacterDefinition;
class UAetherPartyComponent;
class UAetherBaseAttributeSet;
class UAetherAbilitySystemComponent;

UCLASS()
class AETHER_API AAetherPartyMemberStateActor : public AActor
{
	GENERATED_BODY()

public:
	AAetherPartyMemberStateActor();
	// Uses directly referenced configuration once; invalid input or repeated setup returns false.
	bool InitializeCharacterRuntime(UAetherCharacterDefinition* Definition, AActor* AvatarActor,
		UAetherPartyComponent* InParty = nullptr);
	UAetherAbilitySystemComponent* GetAetherAbilitySystemComponent() const { return AetherASC; }
	UAetherPartyComponent* GetPartyComponent() const { return Party.Get(); }

	
	USkeletalMesh* GetSkeletalMesh() const;
	TSubclassOf<UAnimInstance> GetAnimInstanceClass() const;

private:
	// Null for a standalone (single-member NPC) runtime; never resolved through a global player.
	UPROPERTY()
	TWeakObjectPtr<UAetherPartyComponent> Party;
	UPROPERTY()
	TObjectPtr<UAetherCharacterDefinition> CharacterDefinition;
	UPROPERTY()
	TObjectPtr<UAetherAbilitySystemComponent> AetherASC;
	UPROPERTY()
	TObjectPtr<UAetherBaseAttributeSet> BaseAttributeSet;
	
	FAetherAbilitySet_GrantedHandles GrantedAbilitySetHandles;
	
	
	
};
