// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPartyMemberStateActor.h"

#include "Aether/AetherCharacterAssetSubsystem.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AetherCharacterDefinition.h"
#include "Aether/AbilitySystem/AttributeSet/AetherBaseAttributeSet.h"


AAetherPartyMemberStateActor::AAetherPartyMemberStateActor()
{
	PrimaryActorTick.bCanEverTick = false;
	AetherASC = CreateDefaultSubobject<UAetherAbilitySystemComponent>("AetherASC");
	BaseAttributeSet = CreateDefaultSubobject<UAetherBaseAttributeSet>("BaseAttributeSet");
}

void AAetherPartyMemberStateActor::InitializePartyMemberStateActor(FPrimaryAssetId Id, AActor* AvatarActor)
{
	AetherASC->InitAbilityActorInfo(this, AvatarActor);
	CharacterDefinitionId = Id;
	UAetherCharacterDefinition* CharacterData = GetCharacterData();
	if (!CharacterData)
	{
		return;
	}

	for (const TSoftObjectPtr<UAetherAbilitySet>& AbilitySetPtr : CharacterData->AbilitySets)
	{
		if (UAetherAbilitySet* AbilitySet = AbilitySetPtr.LoadSynchronous())
		{
			AbilitySet->InitializeAbilitySystem(AetherASC, GrantedAbilitySetHandles, nullptr);
		}
	}
}

USkeletalMesh* AAetherPartyMemberStateActor::GetSkeletalMesh() const
{
	UAetherCharacterDefinition* CharacterData = GetCharacterData();
	return CharacterData ? CharacterData->DisplayMesh.LoadSynchronous() : nullptr;
}

TSubclassOf<UAnimInstance> AAetherPartyMemberStateActor::GetAnimInstanceClass() const
{
	UAetherCharacterDefinition* CharacterData = GetCharacterData();
	return CharacterData ? CharacterData->DisplayAnimInstanceClass.LoadSynchronous() : nullptr;
}

UAetherCharacterDefinition* AAetherPartyMemberStateActor::GetCharacterData() const
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	UAetherCharacterAssetSubsystem* CharacterAssetSubsystem = GameInstance->GetSubsystem<UAetherCharacterAssetSubsystem>();
	return CharacterAssetSubsystem ? CharacterAssetSubsystem->GetLoadedCharacterDefinition(CharacterDefinitionId) : nullptr;
}
