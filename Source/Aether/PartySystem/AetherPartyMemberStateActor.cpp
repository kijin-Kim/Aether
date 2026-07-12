// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPartyMemberStateActor.h"

#include "Aether/AetherCharacterDatabase.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AetherCharacterData.h"
#include "Aether/AbilitySystem/AttributeSet/AetherBaseAttributeSet.h"


AAetherPartyMemberStateActor::AAetherPartyMemberStateActor()
{
	PrimaryActorTick.bCanEverTick = true;
	AetherASC = CreateDefaultSubobject<UAetherAbilitySystemComponent>("AetherASC");
	BaseAttributeSet = CreateDefaultSubobject<UAetherBaseAttributeSet>("BaseAttributeSet");
}

void AAetherPartyMemberStateActor::InitializePartyMemberStateActor(FName NewCharacterId, AActor* AvatarActor)
{
	AetherASC->InitAbilityActorInfo(this, AvatarActor);
	CharacterId = NewCharacterId;
	UAetherCharacterData* CharacterData = GetCharacterData();
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
	UAetherCharacterData* CharacterData = GetCharacterData();
	return CharacterData ? CharacterData->Mesh.LoadSynchronous() : nullptr;
}

TSubclassOf<UAnimInstance> AAetherPartyMemberStateActor::GetAnimInstanceClass() const
{
	UAetherCharacterData* CharacterData = GetCharacterData();
	return CharacterData ? CharacterData->AnimInstanceClass.LoadSynchronous() : nullptr;
}

UAetherCharacterData* AAetherPartyMemberStateActor::GetCharacterData() const
{
	UAetherCharacterDatabase* DB = GetGameInstance()->GetSubsystem<UAetherCharacterDatabase>();
	return DB ? DB->GetCharacterByID(CharacterId) : nullptr;
}
