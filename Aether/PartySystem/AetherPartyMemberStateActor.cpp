// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPartyMemberStateActor.h"

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
	UAetherCharacterDefinition* Definition, AActor* AvatarActor)
{
	if (CharacterDefinition || !IsValid(Definition) || !IsValid(AvatarActor))
	{
		return false;
	}

	CharacterDefinition = Definition;
	// OnGranted abilities may use the avatar immediately.
	SetAvatarActor(AvatarActor);
	for (const TSoftObjectPtr<UAetherAbilitySet>& AbilitySetPtr : CharacterDefinition->AbilitySets)
	{
		if (UAetherAbilitySet* AbilitySet = AbilitySetPtr.LoadSynchronous())
		{
			AbilitySet->InitializeAbilitySystem(AetherASC, GrantedAbilitySetHandles, nullptr);
		}
	}
	return true;
}

void AAetherPartyMemberStateActor::SetAvatarActor(AActor* AvatarActor)
{
	AetherASC->InitAbilityActorInfo(this, AvatarActor);
}

USkeletalMesh* AAetherPartyMemberStateActor::GetSkeletalMesh() const
{
	return CharacterDefinition ? CharacterDefinition->DisplayMesh.LoadSynchronous() : nullptr;
}

TSubclassOf<UAnimInstance> AAetherPartyMemberStateActor::GetAnimInstanceClass() const
{
	return CharacterDefinition ? CharacterDefinition->DisplayAnimInstanceClass.LoadSynchronous() : nullptr;
}
