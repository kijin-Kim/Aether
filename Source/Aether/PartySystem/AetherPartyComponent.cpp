// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPartyComponent.h"

#include "AetherFieldCharacterPawn.h"
#include "AetherPartyAbilitySystemInterface.h"
#include "AetherPartyMemberStateActor.h"
#include "Aether/Aether.h"
#include "Aether/AetherCharacterDatabase.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AetherCharacterData.h"
#include "Aether/AbilitySystem/Abilities/AetherGameplayAbility_SwitchPartySlot.h"
#include "GameFramework/PlayerState.h"


UAetherPartyComponent::UAetherPartyComponent()
{
	PartyCharacterIds = {TEXT("Yuito"), TEXT("Hanabi")};
}

void UAetherPartyComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeParty(PartyCharacterIds);
}


void UAetherPartyComponent::InitializeParty(const TArray<FName>& CharacterIds)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (CharacterIds.IsEmpty())
	{
		UE_LOG(LogAether, Warning, TEXT("InitializeParty skipped: CharacterIds is empty."));
		return;
	}

	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UAetherCharacterDatabase* DB = GameInstance->GetSubsystem<UAetherCharacterDatabase>())
		{
			const TWeakObjectPtr<UAetherPartyComponent> WeakThis(this);
			DB->LoadCharacterDataAsync(CharacterIds, FStreamableDelegate::CreateLambda([WeakThis, CharacterIds]()
			{
				if (UAetherPartyComponent* PartyComponent = WeakThis.Get())
				{
					PartyComponent->SpawnAndSetupCharacters(CharacterIds);
					if (IAetherPartyAbilitySystemInterface* PartyAbilitySystem = PartyComponent->GetOwner<
						IAetherPartyAbilitySystemInterface>())
					{
						UAetherAbilitySystemComponent* PartyASC = PartyAbilitySystem->GetPartyAbilitySystemComponent();
						PartyASC->InitAbilityActorInfo(
							PartyComponent->GetOwner(), PartyComponent->GetFieldCharacterPawn());

						for (const FGameplayTag& PartySwitchTag : AetherGameplayTags::GetPartySwitchTags())
						{
							FGameplayAbilitySpec NewSpec(UAetherGameplayAbility_SwitchPartySlot::StaticClass());
							NewSpec.GetDynamicSpecSourceTags().AddTag(PartySwitchTag);
							PartyASC->GiveAbility(NewSpec);
						}
					}
				}
			}));
		}
	}
}


void UAetherPartyComponent::SpawnAndSetupCharacters(const TArray<FName>& CharacterIds)
{
	APlayerState* PS = GetOwner<APlayerState>();
	APlayerController* PC = PS->GetPlayerController();


	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	UAetherCharacterDatabase* DB = GameInstance ? GameInstance->GetSubsystem<UAetherCharacterDatabase>() : nullptr;


	FTransform SpawnTransform = FTransform::Identity;
	if (PC->StartSpot.Get())
	{
		SpawnTransform = PC->StartSpot->GetActorTransform();
	}
	else if (APawn* ExistingPawn = PC->GetPawn())
	{
		SpawnTransform = ExistingPawn->GetActorTransform();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FieldCharacterPawn = GetWorld()->SpawnActor<AAetherFieldCharacterPawn>(
		AAetherFieldCharacterPawn::StaticClass(), SpawnTransform, SpawnParams);
	PC->Possess(FieldCharacterPawn);


	for (const FName& CharacterId : CharacterIds)
	{
		AAetherPartyMemberStateActor* NewMemberActor = GetWorld()->SpawnActor<AAetherPartyMemberStateActor>();
		NewMemberActor->InitializePartyMemberStateActor(CharacterId, FieldCharacterPawn);
		PartyMemberStateActors.Add(NewMemberActor);
	}
	RequestSwitchPartySlot(0);
}

bool UAetherPartyComponent::RequestSwitchPartySlot(int32 SlotIndex)
{
	if (!PartyMemberStateActors.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogAether, Warning, TEXT("RequestSwitchPartySlot failed: Invalid SlotIndex %d"), SlotIndex);
		return false;
	}

	ActiveSlotIndex = SlotIndex;

	FieldCharacterPawn->GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	FieldCharacterPawn->GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	FieldCharacterPawn->GetMesh()->SetSkeletalMesh(PartyMemberStateActors[SlotIndex]->GetSkeletalMesh());
	FieldCharacterPawn->GetMesh()->SetAnimInstanceClass(PartyMemberStateActors[SlotIndex]->GetAnimInstanceClass());

	return true;
}

UAetherAbilitySystemComponent* UAetherPartyComponent::GetActiveAbilitySystemComponent() const
{
	if (PartyMemberStateActors.IsValidIndex(ActiveSlotIndex))
	{
		return PartyMemberStateActors[ActiveSlotIndex]->GetAetherAbilitySystemComponent();
	}
	return nullptr;
}
