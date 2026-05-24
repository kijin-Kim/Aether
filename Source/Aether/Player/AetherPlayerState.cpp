#include "AetherPlayerState.h"

#include "Aether/Aether.h"
#include "Aether/AetherCharacterDatabase.h"
#include "Aether/AbilitySystem/AetherCharacterData.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/Character/AetherCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	APlayerController* PC = nullptr;
	AAetherCharacter* NewCharacter = nullptr;
	if (!CanSwitchPartySlot(SlotIndex, PC, NewCharacter))
	{
		return;
	}

	const int32 PreviousSlotIndex = ActiveSlotIndex;
	AAetherCharacter* OldCharacter = GetActivePartyCharacter();

	NewCharacter->SetOnField(true);
	CopyPartySwapState(OldCharacter, NewCharacter);
	RefreshPartyControl(PC, OldCharacter, NewCharacter);

	ActiveSlotIndex = SlotIndex;
	OnActivePartySlotChanged.Broadcast(PreviousSlotIndex, ActiveSlotIndex, OldCharacter, NewCharacter);
}

AAetherCharacter* AAetherPlayerState::GetActivePartyCharacter() const
{
	return PartyCharacters.IsValidIndex(ActiveSlotIndex) ? PartyCharacters[ActiveSlotIndex].Get() : nullptr;
}

bool AAetherPlayerState::CanSwitchPartySlot(int32 SlotIndex, APlayerController*& OutPlayerController, AAetherCharacter*& OutNewCharacter) const
{
	OutPlayerController = nullptr;
	OutNewCharacter = nullptr;

	if (!PartyCharacters.IsValidIndex(SlotIndex) || SlotIndex == ActiveSlotIndex)
	{
		return false;
	}

	OutPlayerController = GetPlayerController();
	if (!OutPlayerController)
	{
		return false;
	}

	OutNewCharacter = PartyCharacters[SlotIndex];
	return OutNewCharacter != nullptr;
}

void AAetherPlayerState::CopyPartySwapState(const AAetherCharacter* PreviousCharacter, AAetherCharacter* NewCharacter) const
{
	if (!PreviousCharacter || !NewCharacter)
	{
		return;
	}

	NewCharacter->SetActorTransform(PreviousCharacter->GetActorTransform(), false, nullptr, ETeleportType::TeleportPhysics);

	const UCharacterMovementComponent* PreviousMovement = PreviousCharacter->GetCharacterMovement();
	UCharacterMovementComponent* NewMovement = NewCharacter->GetCharacterMovement();
	if (PreviousMovement && NewMovement)
	{
		NewMovement->SetMovementMode(PreviousMovement->MovementMode, PreviousMovement->CustomMovementMode);
		NewMovement->Velocity = PreviousMovement->Velocity;
	}
}

void AAetherPlayerState::RefreshPartyControl(APlayerController* PlayerController, AAetherCharacter* PreviousCharacter, AAetherCharacter* NewCharacter) const
{
	if (!PlayerController || !NewCharacter)
	{
		return;
	}

	if (PreviousCharacter)
	{
		if (UAetherAbilitySystemComponent* PreviousASC = PreviousCharacter->GetAetherAbilitySystemComponent())
		{
			PreviousASC->ClearInputs();
		}
	}

	if (UAetherAbilitySystemComponent* NewASC = NewCharacter->GetAetherAbilitySystemComponent())
	{
		NewASC->ClearInputs();
	}

	PlayerController->Possess(NewCharacter);
	PlayerController->FlushPressedKeys();

	if (PreviousCharacter)
	{
		PreviousCharacter->SetOnField(false);
	}
}
