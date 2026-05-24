// Fill out your copyright notice in the Description page of Project Settings.
#include "AetherCharacterBase.h"

#include "Aether/AetherCharacterDatabase.h"
#include "Aether/AbilitySystem/AetherAbilitySet.h"
#include "Aether/AbilitySystem/AetherCharacterData.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAetherCharacterBase::AAetherCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AetherASC = CreateDefaultSubobject<UAetherAbilitySystemComponent>("AetherASC");

	bUseControllerRotationYaw = false;

	GetCapsuleComponent()->SetCapsuleHalfHeight(90.0f);
	GetCapsuleComponent()->SetCapsuleRadius(35.0f);

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -89.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));

	WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Weapon");
	WeaponMeshComponent->SetupAttachment(GetMesh(), WeaponSocketName);

	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AAetherCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	WeaponMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
}

void AAetherCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AetherASC->InitAbilityActorInfo(this, this);
}

void AAetherCharacterBase::UnPossessed()
{
	Super::UnPossessed();
	AetherASC->ClearInputs();
}

void AAetherCharacterBase::InitializeFromCharacterData(FName NewCharacterId)
{
	CharacterId = NewCharacterId;

	UAetherCharacterData* CharacterData = GetGameInstance()->GetSubsystem<UAetherCharacterDatabase>()->GetCharacterByID(CharacterId);
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

	if (CharacterData->Mesh)
	{
		GetMesh()->SetSkeletalMesh(CharacterData->Mesh.LoadSynchronous());
	}

	if (CharacterData->AnimInstanceClass)
	{
		GetMesh()->SetAnimInstanceClass(CharacterData->AnimInstanceClass.LoadSynchronous());
	}
}

void AAetherCharacterBase::SetOnField(bool bSetOnField)
{
	bOnField = bSetOnField;
	SetActorHiddenInGame(!bOnField);
	SetActorTickEnabled(bOnField);
	SetActorEnableCollision(bOnField);
}
