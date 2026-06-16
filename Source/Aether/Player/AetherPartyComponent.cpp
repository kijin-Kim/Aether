// Fill out your copyright notice in the Description page of Project Settings.

#include "AetherPartyComponent.h"

#include "AetherPlayerController.h"
#include "Aether/Aether.h"
#include "Aether/AetherCharacterDatabase.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AetherCharacterData.h"
#include "Aether/AbilitySystem/AetherGameplayAbility.h"
#include "Aether/Character/AetherCharacter.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

UAetherPartyComponent::UAetherPartyComponent()
{
	InitialPartyCharacterIds = {TEXT("Yuito"), TEXT("Hanabi")};
}

void UAetherPartyComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoInitializeInitialParty && GetOwner() && !InitialPartyCharacterIds.IsEmpty())
	{
		InitializeParty(InitialPartyCharacterIds);
	}
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
				}
			}));
		}
	}
}

bool UAetherPartyComponent::RequestSwitchPartySlot(int32 SlotIndex)
{
	if (bIsSwitching)
	{
		return false;
	}

	AAetherPlayerController* PlayerController = GetOwningAetherPlayerController();
	AAetherCharacter* NewCharacter = GetPartyCharacterAtSlot(SlotIndex);
	if (!PlayerController || !NewCharacter || SlotIndex == ActiveSlotIndex)
	{
		return false;
	}

	if (!CanSwitchFromCurrentCharacter())
	{
		return false;
	}

	bIsSwitching = true;

	const int32 PreviousSlotIndex = ActiveSlotIndex;
	AAetherCharacter* PreviousCharacter = GetActivePartyCharacter();

	ClearSwapInputs(PreviousCharacter);
	ClearSwapInputs(NewCharacter);
	CancelSwapAbilities(PreviousCharacter);

	NewCharacter->SetOnField(true);

	PlayerController->Possess(NewCharacter);
	PlayerController->FlushPressedKeys();
	CopyPartySwapState(PreviousCharacter, NewCharacter);

	if (UAetherAbilitySystemComponent* NewASC = NewCharacter->GetAetherAbilitySystemComponent())
	{
		NewASC->InitAbilityActorInfo(NewCharacter, NewCharacter);
		NewASC->ClearInputs();
	}

	if (PreviousCharacter)
	{
		PreviousCharacter->SetOnField(false);
	}

	ActiveSlotIndex = SlotIndex;
	bIsSwitching = false;

	OnActivePartySlotChanged.Broadcast(PreviousSlotIndex, ActiveSlotIndex, PreviousCharacter, NewCharacter);
	return true;
}

AAetherCharacter* UAetherPartyComponent::GetActivePartyCharacter() const
{
	return PartyCharacters.IsValidIndex(ActiveSlotIndex) ? PartyCharacters[ActiveSlotIndex].Get() : nullptr;
}

void UAetherPartyComponent::SpawnAndSetupCharacters(const TArray<FName>& CharacterIds)
{
	AAetherPlayerController* PlayerController = GetOwningAetherPlayerController();
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UAetherCharacterDatabase* DB = GameInstance ? GameInstance->GetSubsystem<UAetherCharacterDatabase>() : nullptr;
	if (!PlayerController || !World || !DB)
	{
		return;
	}

	ClearPartyRoster(true);

	TSet<FName> SpawnedCharacterIds;
	for (const FName& CharacterId : CharacterIds)
	{
		if (CharacterId.IsNone() || SpawnedCharacterIds.Contains(CharacterId))
		{
			UE_LOG(LogAether, Warning, TEXT("Party character id is empty or duplicated: %s"), *CharacterId.ToString());
			continue;
		}

		UAetherCharacterData* CharacterData = DB->GetCharacterByID(CharacterId);
		if (!CharacterData)
		{
			UE_LOG(LogAether, Warning, TEXT("Character data not found for ID: %s"), *CharacterId.ToString());
			continue;
		}

		UClass* CharacterClass = CharacterData->CharacterClass.LoadSynchronous();
		if (!CharacterClass || !CharacterClass->IsChildOf(AAetherCharacter::StaticClass()))
		{
			UE_LOG(LogAether, Warning, TEXT("Invalid party CharacterClass for ID: %s"), *CharacterId.ToString());
			continue;
		}

		FTransform SpawnTransform = FTransform::Identity;
		if (PlayerController->StartSpot.Get())
		{
			SpawnTransform = PlayerController->StartSpot->GetActorTransform();
		}
		else if (APawn* ExistingPawn = PlayerController->GetPawn())
		{
			SpawnTransform = ExistingPawn->GetActorTransform();
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = PlayerController;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AAetherCharacter* SpawnedCharacter = World->SpawnActor<AAetherCharacter>(
			CharacterClass, SpawnTransform, SpawnParams);
		if (!SpawnedCharacter)
		{
			UE_LOG(LogAether, Warning, TEXT("Failed to spawn party character for ID: %s"), *CharacterId.ToString());
			continue;
		}

		SpawnedCharacter->InitializeFromCharacterData(CharacterId);
		SpawnedCharacter->SetOnField(false);
		PartyCharacters.Add(SpawnedCharacter);
		SpawnedCharacterIds.Add(CharacterId);
	}

	if (!PartyCharacters.IsEmpty())
	{
		RequestSwitchPartySlot(0);
	}
}

AAetherPlayerController* UAetherPartyComponent::GetOwningAetherPlayerController() const
{
	return Cast<AAetherPlayerController>(GetOwner());
}

AAetherCharacter* UAetherPartyComponent::GetPartyCharacterAtSlot(int32 SlotIndex) const
{
	return PartyCharacters.IsValidIndex(SlotIndex) ? PartyCharacters[SlotIndex].Get() : nullptr;
}

bool UAetherPartyComponent::CanSwitchFromCurrentCharacter() const
{
	if (AAetherCharacter* PreviousCharacter = GetActivePartyCharacter())
	{
		if (UAetherAbilitySystemComponent* PreviousASC = PreviousCharacter->GetAetherAbilitySystemComponent())
		{
			if (PreviousASC->HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy::Block))
			{
				return false;
			}
		}
	}

	return true;
}

void UAetherPartyComponent::CopyPartySwapState(const AAetherCharacter* PreviousCharacter,
                                               AAetherCharacter* NewCharacter) const
{
	if (!PreviousCharacter || !NewCharacter)
	{
		return;
	}

	NewCharacter->SetActorTransform(PreviousCharacter->GetActorTransform(), false, nullptr,
	                                ETeleportType::TeleportPhysics);

	const UCharacterMovementComponent* PreviousMovement = PreviousCharacter->GetCharacterMovement();
	UCharacterMovementComponent* NewMovement = NewCharacter->GetCharacterMovement();
	if (PreviousMovement && NewMovement)
	{
		NewMovement->SetMovementMode(PreviousMovement->MovementMode, PreviousMovement->CustomMovementMode);
		NewMovement->Velocity = PreviousMovement->Velocity;
	}
}

void UAetherPartyComponent::ClearSwapInputs(AAetherCharacter* Character) const
{
	if (Character)
	{
		if (UAetherAbilitySystemComponent* AetherASC = Character->GetAetherAbilitySystemComponent())
		{
			AetherASC->ClearInputs();
		}
	}
}

void UAetherPartyComponent::CancelSwapAbilities(AAetherCharacter* Character) const
{
	if (Character)
	{
		if (UAetherAbilitySystemComponent* AetherASC = Character->GetAetherAbilitySystemComponent())
		{
			AetherASC->CancelActiveAbilitiesWithSwapPolicy(EAetherAbilitySwapPolicy::Cancel);
		}
	}
}

void UAetherPartyComponent::ClearPartyRoster(bool bDestroyExistingCharacters)
{
	for (AAetherCharacter* Character : PartyCharacters)
	{
		if (bDestroyExistingCharacters && IsValid(Character))
		{
			Character->Destroy();
		}
	}

	PartyCharacters.Reset();
	ActiveSlotIndex = INDEX_NONE;
}
