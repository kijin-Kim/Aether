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
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/StrongObjectPtr.h"


UAetherPartyComponent::UAetherPartyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAetherPartyComponent::InitializeParty(const TArray<FPrimaryAssetId>& CharacterIds)
{
	if (bPartyConfigured || !GetOwner() || !GetWorld())
	{
		return;
	}

	TSet<FPrimaryAssetId> SeenCharacterIds;
	for (const FPrimaryAssetId& CharacterId : CharacterIds)
	{
		if (!CharacterId.IsValid() || CharacterId.PrimaryAssetType != FPrimaryAssetType(TEXT("CharacterDefinition"))
			|| SeenCharacterIds.Contains(CharacterId))
		{
			UE_LOG(LogAether, Warning, TEXT("InitializeParty rejected invalid or duplicate character %s."),
			       *CharacterId.ToString());
			return;
		}
		SeenCharacterIds.Add(CharacterId);
	}

	PartyCharacterIds = CharacterIds;
	if (PartyCharacterIds.Num() > MaxPartyMembers)
	{
		UE_LOG(LogAether, Warning,
		       TEXT("InitializeParty received %d characters; only the fixed %d party slots will be used."),
		       PartyCharacterIds.Num(), MaxPartyMembers);
		PartyCharacterIds.SetNum(MaxPartyMembers);
	}
	PartyMemberStateActors.Reset();
	PartyMemberStateActors.SetNum(PartyCharacterIds.Num());
	LoadedPartyDefinitions.SetNum(PartyCharacterIds.Num());
	ActiveSlotIndex = INDEX_NONE;
	bPartyConfigured = true;
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
	
	// Possession can precede PlayerState BeginPlay; adopt an already controlled pawn.
	if (AAetherPlayerState* PS = GetOwner<AAetherPlayerState>())
	{
		if (APlayerController* PC = PS->GetPlayerController())
		{
			AttachFieldPawn(Cast<AAetherFieldCharacterPawn>(PC->GetPawn()));
		}
	}
	TryInitializePartyRuntime();

	for (const FPrimaryAssetId& CharacterId : PartyCharacterIds)
	{
		FOnCharacterDefinitionLoaded OnLoadedDelegate;
		OnLoadedDelegate.BindDynamic(this, &UAetherPartyComponent::HandleInitialCharacterLoaded);
		CharacterAssetSubsystem->LoadCharacterGameplayAndDisplay(CharacterId, MoveTemp(OnLoadedDelegate));
	}
}

void UAetherPartyComponent::AttachFieldPawn(AAetherFieldCharacterPawn* Pawn)
{
	if (!IsValid(Pawn) || Pawn == FieldCharacterPawn)
	{
		return;
	}
	if (FieldCharacterPawn)
	{
		DetachFieldPawn(FieldCharacterPawn);
	}
	FieldCharacterPawn = Pawn;
	if (UAetherAbilitySystemComponent* PartyASC = GetPartyAbilitySystemComponent())
	{
		PartyASC->InitAbilityActorInfo(GetOwner(), Pawn);
	}
	for (AAetherPartyMemberStateActor* Member : PartyMemberStateActors)
	{
		if (Member)
		{
			Member->SetAvatarActor(Pawn);
		}
	}
	TryInitializePartyRuntime();
	if (ActiveSlotIndex != INDEX_NONE)
	{
		ApplyPartyMemberToFieldPawn(ActiveSlotIndex);
	}
}

void UAetherPartyComponent::DetachFieldPawn(AAetherFieldCharacterPawn* Pawn)
{
	if (!Pawn || Pawn != FieldCharacterPawn)
	{
		return;
	}
	FieldCharacterPawn = nullptr;
	if (UAetherAbilitySystemComponent* PartyASC = GetPartyAbilitySystemComponent())
	{
		PartyASC->CancelAllAbilities();
		PartyASC->InitAbilityActorInfo(GetOwner(), nullptr);
	}
	for (AAetherPartyMemberStateActor* Member : PartyMemberStateActors)
	{
		if (Member)
		{
			UAetherAbilitySystemComponent* ASC = Member->GetAetherAbilitySystemComponent();
			ASC->CancelAllAbilities();
			Member->SetAvatarActor(nullptr);
		}
	}
}

void UAetherPartyComponent::TryInitializePartyRuntime()
{
	UAetherAbilitySystemComponent* PartyASC = GetPartyAbilitySystemComponent();
	if (!bPartyConfigured || PartyCharacterIds.IsEmpty() || !IsValid(FieldCharacterPawn)
		|| !GetWorld() || !PartyASC)
	{
		return;
	}

	EnsurePartyAbilitiesGranted(PartyASC);

	for (int32 SlotIndex = 0; SlotIndex < LoadedPartyDefinitions.Num(); ++SlotIndex)
	{
		if (!LoadedPartyDefinitions[SlotIndex] || PartyMemberStateActors[SlotIndex])
		{
			continue;
		}
		AAetherPartyMemberStateActor* Member = GetWorld()->SpawnActor<AAetherPartyMemberStateActor>();
		if (!Member)
		{
			continue;
		}
		PartyMemberStateActors[SlotIndex] = Member;
		if (!Member->InitializeCharacterRuntime(LoadedPartyDefinitions[SlotIndex], FieldCharacterPawn))
		{
			PartyMemberStateActors[SlotIndex] = nullptr;
			Member->Destroy();
			continue;
		}
		LoadedPartyDefinitions[SlotIndex] = nullptr;
		const bool bShouldActivateInitialSlot = SlotIndex == 0 && ActiveSlotIndex == INDEX_NONE;
		if (bShouldActivateInitialSlot)
		{
			if (RequestSwitchPartySlot(0))
			{
				// A successful switch already broadcasts the change.
				continue;
			}
		}
		OnPartyChanged.Broadcast();
	}
}

void UAetherPartyComponent::EnsurePartyAbilitiesGranted(UAetherAbilitySystemComponent* PartyASC)
{
	if (bPartyAbilitiesGranted)
	{
		return;
	}

	// OnGranted can reenter initialization; mark before granting, with the avatar already attached.
	bPartyAbilitiesGranted = true;
	FAetherAbilitySet_GrantedHandles GrantedHandles;
	if (PartyAbilitySet)
	{
		PartyAbilitySet->InitializeAbilitySystem(PartyASC, GrantedHandles, nullptr);
	}

	// 파티 슬롯 교체 Ability 부여
	for (const FGameplayTag& SwitchTag : AetherGameplayTags::GetPartySwitchTags())
	{
		FGameplayAbilitySpec Spec(UAetherGameplayAbility_SwitchPartySlot::StaticClass());
		Spec.GetDynamicSpecSourceTags().AddTag(SwitchTag);
		PartyASC->GiveAbility(Spec);
	}
}

void UAetherPartyComponent::HandleInitialCharacterLoaded(
	FPrimaryAssetId RequestedCharacterId, UAetherCharacterDefinition* LoadedDefinition)
{
	const int32 SlotIndex = PartyCharacterIds.IndexOfByKey(RequestedCharacterId);
	if (!PartyMemberStateActors.IsValidIndex(SlotIndex) || !GetWorld())
	{
		return;
	}
	if (!IsValid(LoadedDefinition) || LoadedDefinition->GetPrimaryAssetId() != RequestedCharacterId)
	{
		UE_LOG(LogAether, Warning, TEXT("HandleInitialCharacterLoaded rejected result for %s."),
		       *RequestedCharacterId.ToString());
		return;
	}
	if (PartyMemberStateActors[SlotIndex])
	{
		return;
	}

	LoadedPartyDefinitions[SlotIndex] = LoadedDefinition;
	TryInitializePartyRuntime();
}

void UAetherPartyComponent::RegisterAndCallPartyChanged(FOnAetherPartyChangedCallback Callback)
{
	if (!Callback.IsBound())
	{
		return;
	}

	OnPartyChanged.AddUnique(Callback);

	if (bPartyConfigured)
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
		|| !bPartyConfigured
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
		const bool bSucceeded = SwapPartySlots(SlotIndex, ExistingSlotIndex);
		OnCompleted.ExecuteIfBound(bSucceeded);
		return bSucceeded;
	}

	return BeginPartyMemberReplacement(SlotIndex, NewCharacterId, MoveTemp(OnCompleted));
}

bool UAetherPartyComponent::SwapPartySlots(int32 SlotIndex, int32 ExistingSlotIndex)
{
	if (!PartyMemberStateActors.IsValidIndex(SlotIndex)
		|| !PartyMemberStateActors.IsValidIndex(ExistingSlotIndex)
		|| !PartyMemberStateActors[SlotIndex]
		|| !PartyMemberStateActors[ExistingSlotIndex])
	{
		return false;
	}

	const bool bAffectsActiveSlot = ActiveSlotIndex == SlotIndex || ActiveSlotIndex == ExistingSlotIndex;
	if (bAffectsActiveSlot)
	{
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
	}

	PartyCharacterIds.Swap(SlotIndex, ExistingSlotIndex);
	PartyMemberStateActors.Swap(SlotIndex, ExistingSlotIndex);
	if (bAffectsActiveSlot && !ApplyPartyMemberToFieldPawn(ActiveSlotIndex))
	{
		PartyCharacterIds.Swap(SlotIndex, ExistingSlotIndex);
		PartyMemberStateActors.Swap(SlotIndex, ExistingSlotIndex);
		return false;
	}

	OnPartyChanged.Broadcast();
	return true;
}

bool UAetherPartyComponent::BeginPartyMemberReplacement(
	int32 SlotIndex, const FPrimaryAssetId& NewCharacterId,
	FOnAetherPartyMemberReplaceCompleted OnCompleted)
{
	auto FailRequest = [&OnCompleted]()
	{
		OnCompleted.ExecuteIfBound(false);
		return false;
	};

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

void UAetherPartyComponent::HandleReplacementCharacterLoaded(
	FPrimaryAssetId RequestedCharacterId, UAetherCharacterDefinition* LoadedDefinition)
{
	if (PendingReplacementSlotIndex == INDEX_NONE || RequestedCharacterId != PendingReplacementCharacterId)
	{
		return;
	}
	const int32 SlotIndex = PendingReplacementSlotIndex;
	if (!IsValid(LoadedDefinition)
		|| LoadedDefinition->GetPrimaryAssetId() != PendingReplacementCharacterId
		|| !PartyMemberStateActors.IsValidIndex(SlotIndex)
		|| !PartyCharacterIds.IsValidIndex(SlotIndex)
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

	if (!NewMemberActor->InitializeCharacterRuntime(LoadedDefinition, FieldCharacterPawn)
		|| !NewMemberActor->GetSkeletalMesh())
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

	FOnAetherPartyMemberReplaceCompleted CompletionCallback = MoveTemp(PendingReplacementCallback);
	PendingReplacementSlotIndex = INDEX_NONE;
	PendingReplacementCharacterId = FPrimaryAssetId();
	PendingReplacementCallback.Unbind();

	if (bReplacingActiveSlot && !RequestSwitchPartySlot(SlotIndex))
	{
		PartyMemberStateActors[SlotIndex] = OldMemberActor;
		PartyCharacterIds[SlotIndex] = OldCharacterId;
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
	if (!PartyMemberStateActors.IsValidIndex(SlotIndex) || !IsValid(PartyMemberStateActors[SlotIndex]))
	{
		UE_LOG(LogAether, Warning, TEXT("RequestSwitchPartySlot failed: Invalid SlotIndex %d"), SlotIndex);
		return false;
	}

	if (!IsValid(FieldCharacterPawn) || !IsValid(FieldCharacterPawn->GetMesh()))
	{
		return false;
	}

	const TWeakObjectPtr<AAetherFieldCharacterPawn> PreparedPawn = FieldCharacterPawn;
	const TWeakObjectPtr<AAetherPartyMemberStateActor> TargetMember = PartyMemberStateActors[SlotIndex];
	// Prepare appearance before cancellation; loading failure must not interrupt the current ability.
	const TStrongObjectPtr<USkeletalMesh> PreparedMesh(TargetMember->GetSkeletalMesh());
	if (!PreparedMesh.IsValid())
	{
		return false;
	}
	const TStrongObjectPtr<UClass> PreparedAnimClass(TargetMember->GetAnimInstanceClass().Get());
	const int32 PreviousActiveSlotIndex = ActiveSlotIndex;

	UAetherAbilitySystemComponent* ActiveASC = GetActiveAbilitySystemComponent();
	const TWeakObjectPtr<UAetherAbilitySystemComponent> PreviousActiveASC = ActiveASC;
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

	// Ability end callbacks can change the party or its pawn before cancellation returns.
	if (!PreparedPawn.IsValid() || FieldCharacterPawn != PreparedPawn.Get()
		|| !IsValid(PreparedPawn->GetMesh())
		|| !TargetMember.IsValid() || !PartyMemberStateActors.IsValidIndex(SlotIndex)
		|| PartyMemberStateActors[SlotIndex] != TargetMember.Get()
		|| ActiveSlotIndex != PreviousActiveSlotIndex
		|| GetActiveAbilitySystemComponent() != PreviousActiveASC.Get())
	{
		return false;
	}

	if (!PreparedPawn->ApplyCharacterAppearance(PreparedMesh.Get(), PreparedAnimClass.Get()))
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

	return FieldCharacterPawn->ApplyCharacterAppearance(SkeletalMesh, MemberStateActor->GetAnimInstanceClass());
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
