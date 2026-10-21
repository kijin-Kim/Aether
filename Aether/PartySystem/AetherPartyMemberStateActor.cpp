// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPartyMemberStateActor.h"
#include "AetherPartyComponent.h"

#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AetherCharacterDefinition.h"
#include "Aether/AbilitySystem/AttributeSet/AetherBaseAttributeSet.h"


AAetherPartyMemberStateActor::AAetherPartyMemberStateActor()
{
	PrimaryActorTick.bCanEverTick = false;
	AetherASC = CreateDefaultSubobject<UAetherAbilitySystemComponent>("AetherASC");
	BaseAttributeSet = CreateDefaultSubobject<UAetherBaseAttributeSet>("BaseAttributeSet");
}

bool AAetherPartyMemberStateActor::InitializeCharacterRuntime(
	UAetherCharacterDefinition* Definition, AActor* AvatarActor, UAetherPartyComponent* InParty)
{
	if (CharacterDefinition || !IsValid(Definition) || !IsValid(AvatarActor))
	{
		return false;
	}

	CharacterDefinition = Definition;
	Party = InParty;
	// OnGranted abilities may use the avatar immediately.
	AetherASC->InitAbilityActorInfo(this, AvatarActor);
	for (UAetherAbilitySet* AbilitySet : CharacterDefinition->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->InitializeAbilitySystem(AetherASC, GrantedAbilitySetHandles, nullptr);
		}
	}
	return true;
}

USkeletalMesh* AAetherPartyMemberStateActor::GetSkeletalMesh() const
{
	return CharacterDefinition ? CharacterDefinition->DisplayMesh.Get() : nullptr;
}

TSubclassOf<UAnimInstance> AAetherPartyMemberStateActor::GetAnimInstanceClass() const
{
	return CharacterDefinition ? CharacterDefinition->DisplayAnimInstanceClass : nullptr;
}
