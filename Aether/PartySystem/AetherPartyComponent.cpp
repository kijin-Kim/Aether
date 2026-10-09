// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPartyComponent.h"

#include "AetherFieldCharacterPawn.h"
#include "AetherPartyMemberStateActor.h"
#include "Aether/Aether.h"
#include "Aether/AetherCharacterAssetSubsystem.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AetherCharacterDefinition.h"
#include "Aether/AbilitySystem/Abilities/AetherGameplayAbility_SwitchPartySlot.h"
#include "Aether/Player/AetherPlayerState.h"


UAetherPartyComponent::UAetherPartyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAetherPartyComponent::InitializeParty(const TArray<FPrimaryAssetId>& CharacterIds)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	PartyCharacterIds = CharacterIds;
	if (PartyCharacterIds.Num() > MaxPartyMembers)
	{
		UE_LOG(LogAether, Warning,
		       TEXT("InitializeParty received %d characters; only the fixed %d party slots will be used."),
		       PartyCharacterIds.Num(), MaxPartyMembers);
		PartyCharacterIds.SetNum(MaxPartyMembers);
	}
	PendingPartyCharacterIds = PartyCharacterIds;
	PartyMemberStateActors.Reset();
	PartyMemberStateActors.SetNum(PartyCharacterIds.Num());
	ActiveSlotIndex = INDEX_NONE;
	bPartyInitialized = true;
	OnPartyChanged.Broadcast();

	if (PartyCharacterIds.IsEmpty())
	{
		UE_LOG(LogAether, Warning, TEXT("InitializeParty skipped: CharacterIds is empty."));
		return;
	}


	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UAetherCharacterAssetSubsystem* CharacterAssetSubsystem = GameInstance->GetSubsystem<
		UAetherCharacterAssetSubsystem>();
	if (!CharacterAssetSubsystem)
	{
		return;
	}
	
	if (!SpawnFieldCharacterPawn())
	{
		return;
	}

	if (AAetherPlayerState* PlayerState = GetOwner<AAetherPlayerState>())
	{
		UAetherAbilitySystemComponent* PartyASC = PlayerState->GetPartyASC();
		if (!PartyASC)
		{
			return;
		}

		PartyASC->InitAbilityActorInfo(GetOwner(), FieldCharacterPawn);
		FAetherAbilitySet_GrantedHandles PartyAbilitySetHandles;
		if (PartyAbilitySet)
		{
			PartyAbilitySet->InitializeAbilitySystem(PartyASC, PartyAbilitySetHandles, nullptr);
		}

		for (const FGameplayTag& PartySwitchTag : AetherGameplayTags::GetPartySwitchTags())
		{
			FGameplayAbilitySpec NewSpec(UAetherGameplayAbility_SwitchPartySlot::StaticClass());
			NewSpec.GetDynamicSpecSourceTags().AddTag(PartySwitchTag);
			PartyASC->GiveAbility(NewSpec);
		}
	}

	for (const FPrimaryAssetId& CharacterId : PartyCharacterIds)
	{
		FOnCharacterDefinitionLoaded OnLoadedDelegate;
		OnLoadedDelegate.BindDynamic(this, &UAetherPartyComponent::AddPartyMemberStateActor);
		CharacterAssetSubsystem->LoadCharacterGameplayAndDisplay(CharacterId, MoveTemp(OnLoadedDelegate));
	}
}

bool UAetherPartyComponent::SpawnFieldCharacterPawn()
{
	AAetherPlayerState* PS = GetOwner<AAetherPlayerState>();
	if (!PS)
	{
		return false;
	}

	APlayerController* PC = PS->GetPlayerController();
	if (!PC || !GetWorld())
	{
		return false;
	}

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
	if (!FieldCharacterPawn)
	{
		return false;
	}

	PC->Possess(FieldCharacterPawn);
	return true;
}

void UAetherPartyComponent::AddPartyMemberStateActor(UAetherCharacterDefinition* LoadedDefinition)
{
	if (!LoadedDefinition || !GetWorld() || !FieldCharacterPawn)
	{
		UE_LOG(LogAether, Warning, TEXT("AddPartyMemberStateActor skipped: character definition failed to load."));
		return;
	}

	const FPrimaryAssetId CharacterId = LoadedDefinition->GetPrimaryAssetId();
	const int32 SlotIndex = PendingPartyCharacterIds.IndexOfByKey(CharacterId);
	if (!PartyMemberStateActors.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogAether, Warning, TEXT("AddPartyMemberStateActor skipped: unexpected character %s."),
		       *CharacterId.ToString());
		return;
	}

	AAetherPartyMemberStateActor* NewMemberActor = GetWorld()->SpawnActor<AAetherPartyMemberStateActor>();
	if (!NewMemberActor)
	{
		return;
	}

	NewMemberActor->InitializePartyMemberStateActor(CharacterId, FieldCharacterPawn);
	PartyMemberStateActors[SlotIndex] = NewMemberActor;

	if (SlotIndex != 0 || !RequestSwitchPartySlot(0))
	{
		OnPartyChanged.Broadcast();
	}
}

void UAetherPartyComponent::RegisterAndCallPartyChanged(FOnAetherPartyChangedCallback Callback)
{
	if (!Callback.IsBound())
	{
		return;
	}

	OnPartyChanged.AddUnique(Callback);

	if (bPartyInitialized)
	{
		Callback.Execute();
	}
}

bool UAetherPartyComponent::RequestReplacePartyMember(
	int32 SlotIndex,
	const FPrimaryAssetId& NewCharacterId,
	FOnAetherPartyMemberReplaceCompleted OnCompleted)
{
	auto FailRequest = [&OnCompleted]()
	{
		OnCompleted.ExecuteIfBound(false);
		return false;
	};

	if (PendingReplacementSlotIndex != INDEX_NONE
		|| !bPartyInitialized
		|| !PartyCharacterIds.IsValidIndex(SlotIndex)
		|| SlotIndex >= MaxPartyMembers
		|| !NewCharacterId.IsValid())
	{
		return FailRequest();
	}

	if (PartyCharacterIds[SlotIndex] == NewCharacterId)
	{
		OnCompleted.ExecuteIfBound(true);
		return true;
	}

	const int32 ExistingSlotIndex = PartyCharacterIds.IndexOfByKey(NewCharacterId);
	if (ExistingSlotIndex != INDEX_NONE)
	{
		if (!PartyMemberStateActors.IsValidIndex(SlotIndex)
			|| !PartyMemberStateActors.IsValidIndex(ExistingSlotIndex)
			|| !PartyMemberStateActors[SlotIndex]
			|| !PartyMemberStateActors[ExistingSlotIndex]
			|| !PendingPartyCharacterIds.IsValidIndex(SlotIndex)
			|| !PendingPartyCharacterIds.IsValidIndex(ExistingSlotIndex))
		{
			return FailRequest();
		}

		const bool bAffectsActiveSlot =
			ActiveSlotIndex == SlotIndex || ActiveSlotIndex == ExistingSlotIndex;
		if (bAffectsActiveSlot)
		{
			UAetherAbilitySystemComponent* ActiveASC = GetActiveAbilitySystemComponent();
			UAetherAbilitySystemComponent* PartyASC = GetPartyAbilitySystemComponent();
			if ((ActiveASC && ActiveASC->HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy::Block))
				|| (PartyASC && PartyASC->HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy::Block)))
			{
				return FailRequest();
			}

			if (ActiveASC)
			{
				ActiveASC->CancelActiveAbilitiesWithSwapPolicy(EAetherAbilitySwapPolicy::Cancel);
			}
		}

		PartyCharacterIds.Swap(SlotIndex, ExistingSlotIndex);
		PendingPartyCharacterIds.Swap(SlotIndex, ExistingSlotIndex);
		PartyMemberStateActors.Swap(SlotIndex, ExistingSlotIndex);

		if (bAffectsActiveSlot && !ApplyPartyMemberToFieldPawn(ActiveSlotIndex))
		{
			PartyCharacterIds.Swap(SlotIndex, ExistingSlotIndex);
			PendingPartyCharacterIds.Swap(SlotIndex, ExistingSlotIndex);
			PartyMemberStateActors.Swap(SlotIndex, ExistingSlotIndex);
			return FailRequest();
		}

		OnPartyChanged.Broadcast();
		OnCompleted.ExecuteIfBound(true);
		return true;
	}

	if (!GetWorld() || !FieldCharacterPawn)
	{
		return FailRequest();
	}

	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	UAetherCharacterAssetSubsystem* CharacterAssetSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UAetherCharacterAssetSubsystem>() : nullptr;
	if (!CharacterAssetSubsystem)
	{
		return FailRequest();
	}

	PendingReplacementSlotIndex = SlotIndex;
	PendingReplacementCharacterId = NewCharacterId;
	PendingReplacementCallback = MoveTemp(OnCompleted);

	FOnCharacterDefinitionLoaded OnLoadedDelegate;
	OnLoadedDelegate.BindDynamic(this, &UAetherPartyComponent::HandleReplacementCharacterLoaded);
	CharacterAssetSubsystem->LoadCharacterGameplayAndDisplay(NewCharacterId, MoveTemp(OnLoadedDelegate));
	return true;
}

void UAetherPartyComponent::HandleReplacementCharacterLoaded(UAetherCharacterDefinition* LoadedDefinition)
{
	const int32 SlotIndex = PendingReplacementSlotIndex;
	if (!LoadedDefinition
		|| LoadedDefinition->GetPrimaryAssetId() != PendingReplacementCharacterId
		|| !PartyMemberStateActors.IsValidIndex(SlotIndex)
		|| !PendingPartyCharacterIds.IsValidIndex(SlotIndex)
		|| !GetWorld()
		|| !FieldCharacterPawn)
	{
		CompletePendingReplacement(false);
		return;
	}

	AAetherPartyMemberStateActor* NewMemberActor = GetWorld()->SpawnActor<AAetherPartyMemberStateActor>();
	if (!NewMemberActor)
	{
		CompletePendingReplacement(false);
		return;
	}

	NewMemberActor->InitializePartyMemberStateActor(PendingReplacementCharacterId, FieldCharacterPawn);
	if (!NewMemberActor->GetSkeletalMesh())
	{
		NewMemberActor->Destroy();
		CompletePendingReplacement(false);
		return;
	}

	AAetherPartyMemberStateActor* OldMemberActor = PartyMemberStateActors[SlotIndex];
	const bool bReplacingActiveSlot = ActiveSlotIndex == SlotIndex;
	UAetherAbilitySystemComponent* OldActiveASC =
		bReplacingActiveSlot && OldMemberActor ? OldMemberActor->GetAetherAbilitySystemComponent() : nullptr;
	UAetherAbilitySystemComponent* PartyASC = GetPartyAbilitySystemComponent();

	if ((OldActiveASC && OldActiveASC->HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy::Block))
		|| (PartyASC && PartyASC->HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy::Block)))
	{
		NewMemberActor->Destroy();
		CompletePendingReplacement(false);
		return;
	}

	if (OldActiveASC)
	{
		OldActiveASC->CancelActiveAbilitiesWithSwapPolicy(EAetherAbilitySwapPolicy::Cancel);
	}

	const FPrimaryAssetId NewCharacterId = PendingReplacementCharacterId;
	const FPrimaryAssetId OldCharacterId = PartyCharacterIds[SlotIndex];
	PartyMemberStateActors[SlotIndex] = NewMemberActor;
	PartyCharacterIds[SlotIndex] = NewCharacterId;
	PendingPartyCharacterIds[SlotIndex] = NewCharacterId;

	FOnAetherPartyMemberReplaceCompleted CompletionCallback = MoveTemp(PendingReplacementCallback);
	PendingReplacementSlotIndex = INDEX_NONE;
	PendingReplacementCharacterId = FPrimaryAssetId();
	PendingReplacementCallback.Unbind();

	if (bReplacingActiveSlot && !RequestSwitchPartySlot(SlotIndex))
	{
		PartyMemberStateActors[SlotIndex] = OldMemberActor;
		PartyCharacterIds[SlotIndex] = OldCharacterId;
		PendingPartyCharacterIds[SlotIndex] = OldCharacterId;
		NewMemberActor->Destroy();
		CompletionCallback.ExecuteIfBound(false);
		return;
	}

	if (OldMemberActor)
	{
		OldMemberActor->Destroy();
	}

	if (!bReplacingActiveSlot)
	{
		OnPartyChanged.Broadcast();
	}

	CompletionCallback.ExecuteIfBound(true);
}

void UAetherPartyComponent::CompletePendingReplacement(bool bSucceeded)
{
	FOnAetherPartyMemberReplaceCompleted CompletionCallback = MoveTemp(PendingReplacementCallback);
	PendingReplacementSlotIndex = INDEX_NONE;
	PendingReplacementCharacterId = FPrimaryAssetId();
	PendingReplacementCallback.Unbind();

	CompletionCallback.ExecuteIfBound(bSucceeded);
}

bool UAetherPartyComponent::RequestSwitchPartySlot(int32 SlotIndex)
{
	if (!PartyMemberStateActors.IsValidIndex(SlotIndex) || !PartyMemberStateActors[SlotIndex])
	{
		UE_LOG(LogAether, Warning, TEXT("RequestSwitchPartySlot failed: Invalid SlotIndex %d"), SlotIndex);
		return false;
	}

	if (!FieldCharacterPawn || !FieldCharacterPawn->GetMesh())
	{
		return false;
	}

	UAetherAbilitySystemComponent* ActiveASC = GetActiveAbilitySystemComponent();
	UAetherAbilitySystemComponent* PartyASC = GetPartyAbilitySystemComponent();

	if ((ActiveASC && ActiveASC->HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy::Block))
		|| (PartyASC && PartyASC->HasActiveAbilityWithSwapPolicy(EAetherAbilitySwapPolicy::Block)))
	{
		return false;
	}

	if (ActiveASC)
	{
		ActiveASC->CancelActiveAbilitiesWithSwapPolicy(EAetherAbilitySwapPolicy::Cancel);
	}

	if (!ApplyPartyMemberToFieldPawn(SlotIndex))
	{
		return false;
	}

	ActiveSlotIndex = SlotIndex;
	OnPartyChanged.Broadcast();
	return true;
}

bool UAetherPartyComponent::ApplyPartyMemberToFieldPawn(const int32 SlotIndex)
{
	if (!PartyMemberStateActors.IsValidIndex(SlotIndex)
		|| !PartyMemberStateActors[SlotIndex]
		|| !FieldCharacterPawn
		|| !FieldCharacterPawn->GetMesh())
	{
		return false;
	}

	AAetherPartyMemberStateActor* MemberStateActor = PartyMemberStateActors[SlotIndex];
	USkeletalMesh* SkeletalMesh = MemberStateActor->GetSkeletalMesh();
	if (!SkeletalMesh)
	{
		return false;
	}

	FieldCharacterPawn->GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	FieldCharacterPawn->GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	FieldCharacterPawn->GetMesh()->SetSkeletalMesh(SkeletalMesh);
	FieldCharacterPawn->GetMesh()->SetAnimInstanceClass(MemberStateActor->GetAnimInstanceClass());
	return true;
}

UAetherAbilitySystemComponent* UAetherPartyComponent::GetPartyAbilitySystemComponent() const
{
	if (const AAetherPlayerState* PlayerState = GetOwner<AAetherPlayerState>())
	{
		return PlayerState->GetPartyASC();
	}
	return nullptr;
}

UAetherAbilitySystemComponent* UAetherPartyComponent::GetActiveAbilitySystemComponent() const
{
	if (PartyMemberStateActors.IsValidIndex(ActiveSlotIndex) && PartyMemberStateActors[ActiveSlotIndex])
	{
		return PartyMemberStateActors[ActiveSlotIndex]->GetAetherAbilitySystemComponent();
	}
	return nullptr;
}
