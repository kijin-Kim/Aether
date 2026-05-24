#include "AetherPlayerState.h"

#include "Aether/Aether.h"
#include "Aether/AetherCharacterDatabase.h"
#include "Aether/AbilitySystem/AetherCharacterData.h"
#include "Aether/Character/AetherCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"


void AAetherPlayerState::BeginPlay()
{
	Super::BeginPlay();
	UAetherCharacterDatabase* DB = GetGameInstance()->GetSubsystem<UAetherCharacterDatabase>();
	TArray<FName> Party = {"Yuito", "Hanabi"};
	DB->LoadCharacterDataAsync(Party,
	                           FStreamableDelegate::CreateLambda([this, Party]()
	                           {
		                           SpawnAndSetupCharacter(Party);
	                           }));
}

void AAetherPlayerState::SpawnAndSetupCharacter(const TArray<FName>& CharacterIds)
{
	for (const FName& CharacterId : CharacterIds)
	{
		UAetherCharacterDatabase* DB = GetGameInstance()->GetSubsystem<UAetherCharacterDatabase>();
		UAetherCharacterData* CharacterData = DB->GetCharacterByID(CharacterId);
		if (!CharacterData)
		{
			UE_LOG(LogAether, Warning, TEXT("Character data not found for ID: %s"), *CharacterId.ToString());
			return;
		}

		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogAether, Warning, TEXT("World not found"));
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		FTransform SpawnTransform = FTransform::Identity;
		APlayerController* PC = GetPlayerController();
		if (PC && PC->StartSpot.Get())
		{
			SpawnTransform = PC->StartSpot->GetActorTransform();
		}

		if (AAetherCharacter* SpawnedCharacter = World->SpawnActor<AAetherCharacter>(
			CharacterData->CharacterClass.LoadSynchronous(), SpawnTransform, SpawnParams))
		{
			SpawnedCharacter->InitializeFromCharacterData(CharacterId);
			SpawnedCharacter->SetOnField(false);
			PartyCharacters.Add(SpawnedCharacter);
			UE_LOG(LogAether, Log, TEXT("Successfully spawned character for ID: %s"), *CharacterId.ToString());
		}
		else
		{
			UE_LOG(LogAether, Warning,
			       TEXT("Failed to spawn player character for ID: %s. " "Check that CharacterClass in DataAsset is AAetherCharacter-derived, "
				       "not AAetherAICharacter."), *CharacterId.ToString());
		}
	}

	if (!PartyCharacters.IsEmpty())
	{
		AuthSwitchPartySlot(0);
	}
}

void AAetherPlayerState::AuthSwitchPartySlot(int32 SlotIndex)
{
	check(HasAuthority());

	if (!PartyCharacters.IsValidIndex(SlotIndex))
	{
		return;
	}
	if (SlotIndex == ActiveSlotIndex)
	{
		return;
	}

	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}

	AAetherCharacter* NewCharacter = PartyCharacters[SlotIndex];
	if (!NewCharacter)
	{
		return;
	}

	AAetherCharacter* OldCharacter = (ActiveSlotIndex != INDEX_NONE) ? PartyCharacters[ActiveSlotIndex] : nullptr;

	if (OldCharacter)
	{
		NewCharacter->SetActorTransform(OldCharacter->GetActorTransform());
	}

	NewCharacter->SetOnField(true);

	PC->Possess(NewCharacter);

	if (OldCharacter)
	{
		OldCharacter->SetOnField(false);
	}

	ActiveSlotIndex = SlotIndex;
}
