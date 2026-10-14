// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherPartyComponent.h"

#include "AetherFieldCharacterPawn.h"
#include "AetherPartyMemberStateActor.h"
#include "Aether/Aether.h"
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

	if (PartyCharacterIds.IsEmpty())
	{
		UE_LOG(LogAether, Warning, TEXT("InitializeParty skipped: CharacterIds is empty."));
		return;
	}

	if (PartyCharacterIds.Num() > MaxPartyMembers)
	{
		UE_LOG(LogAether, Warning,
		       TEXT("InitializeParty received %d characters; only the fixed %d party slots will be used."),
		       PartyCharacterIds.Num(), MaxPartyMembers);
		PartyCharacterIds.SetNum(MaxPartyMembers);
	}

	PartyMemberStateActors.Reset();
	PartyMemberStateActors.SetNum(PartyCharacterIds.Num());
	ActiveSlotIndex = INDEX_NONE;
	bPartyConfigured = true;
	OnPartyChanged.Broadcast();


	if (AAetherPlayerState* PS = GetOwner<AAetherPlayerState>())
	{
		if (APlayerController* PC = PS->GetPlayerController())
		{
			InitializeFieldPawn(Cast<AAetherFieldCharacterPawn>(PC->GetPawn()));
		}
	}

	TryInitializePartyRuntime();
}

void UAetherPartyComponent::InitializeFieldPawn(AAetherFieldCharacterPawn* Pawn)
{
	if (!IsValid(Pawn) || FieldCharacterPawn)
	{
		return;
	}

	FieldCharacterPawn = Pawn;
	UAetherAbilitySystemComponent* PartyASC = GetPartyAbilitySystemComponent();
	if (!PartyASC)
	{
		return;
	}

	PartyASC->InitAbilityActorInfo(GetOwner(), FieldCharacterPawn);
	FAetherAbilitySet_GrantedHandles GrantedHandles;
	if (PartyAbilitySet)
	{
		PartyAbilitySet->InitializeAbilitySystem(PartyASC, GrantedHandles, nullptr);
	}

	for (const FGameplayTag& SwitchTag : AetherGameplayTags::GetPartySwitchTags())
	{
		FGameplayAbilitySpec Spec(UAetherGameplayAbility_SwitchPartySlot::StaticClass());
		Spec.GetDynamicSpecSourceTags().AddTag(SwitchTag);
		PartyASC->GiveAbility(Spec);
	}

	TryInitializePartyRuntime();
}

void UAetherPartyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAetherAbilitySystemComponent* PartyASC = GetPartyAbilitySystemComponent())
	{
		PartyASC->CancelAllAbilities();
		PartyASC->ClearAllAbilityInputs();
		PartyASC->ClearActorInfo();
	}
	for (AAetherPartyMemberStateActor* Member : PartyMemberStateActors)
	{
		if (Member)
		{
			UAetherAbilitySystemComponent* ASC = Member->GetAetherAbilitySystemComponent();
			ASC->CancelAllAbilities();
			ASC->ClearAllAbilityInputs();
			ASC->ClearActorInfo();
		}
	}
	FieldCharacterPawn = nullptr;
	Super::EndPlay(EndPlayReason);
}

void UAetherPartyComponent::TryInitializePartyRuntime()
{
	UAetherAbilitySystemComponent* PartyASC = GetPartyAbilitySystemComponent();
	if (!bPartyConfigured || PartyCharacterIds.IsEmpty() || !IsValid(FieldCharacterPawn)
		|| !GetWorld() || !PartyASC)
	{
		return;
	}

	const AAetherPlayerState* PS = GetOwner<AAetherPlayerState>();
	if (!PS)
	{
		return;
	}
	for (int32 SlotIndex = 0; SlotIndex < PartyCharacterIds.Num(); ++SlotIndex)
	{
		if (PartyMemberStateActors[SlotIndex])
		{
			continue;
		}
		UAetherCharacterDefinition* Definition = PS->GetCharacterDefinition(PartyCharacterIds[SlotIndex]);
		if (!Definition)
		{
			UE_LOG(LogAether, Warning, TEXT("No configured Definition for %s."), *PartyCharacterIds[SlotIndex].ToString());
			continue;
		}
		AAetherPartyMemberStateActor* Member = GetWorld()->SpawnActor<AAetherPartyMemberStateActor>();
		if (!Member)
		{
			continue;
		}
		// Register before granting abilities, whose callbacks can reenter party initialization.
		PartyMemberStateActors[SlotIndex] = Member;
		if (!Member->InitializeCharacterRuntime(Definition, FieldCharacterPawn))
		{
			PartyMemberStateActors[SlotIndex] = nullptr;
			Member->Destroy();
			continue;
		}
		if (SlotIndex == 0 && ActiveSlotIndex == INDEX_NONE && RequestSwitchPartySlot(0))
		{
			continue;
		}
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

	if (!bPartyConfigured
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

	const AAetherPlayerState* PS = GetOwner<AAetherPlayerState>();
	UAetherCharacterDefinition* Definition = PS ? PS->GetCharacterDefinition(NewCharacterId) : nullptr;
	const bool bSucceeded = Definition && ReplacePartyMember(SlotIndex, Definition);
	OnCompleted.ExecuteIfBound(bSucceeded);
	return bSucceeded;
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

bool UAetherPartyComponent::ReplacePartyMember(int32 SlotIndex, UAetherCharacterDefinition* Definition)
{
	if (!GetWorld() || !IsValid(FieldCharacterPawn) || !IsValid(Definition)
		|| !PartyMemberStateActors.IsValidIndex(SlotIndex))
	{
		return false;
	}
	AAetherPartyMemberStateActor* NewMemberActor = GetWorld()->SpawnActor<AAetherPartyMemberStateActor>();
	if (!NewMemberActor)
	{
		return false;
	}

	if (!NewMemberActor->InitializeCharacterRuntime(Definition, FieldCharacterPawn)
		|| !NewMemberActor->GetSkeletalMesh())
	{
		NewMemberActor->Destroy();
		return false;
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
		return false;
	}

	if (OldActiveASC)
	{
		OldActiveASC->CancelActiveAbilitiesWithSwapPolicy(EAetherAbilitySwapPolicy::Cancel);
	}

	const FPrimaryAssetId NewCharacterId = Definition->GetPrimaryAssetId();
	const FPrimaryAssetId OldCharacterId = PartyCharacterIds[SlotIndex];
	PartyMemberStateActors[SlotIndex] = NewMemberActor;
	PartyCharacterIds[SlotIndex] = NewCharacterId;

	if (bReplacingActiveSlot && !RequestSwitchPartySlot(SlotIndex))
	{
		PartyMemberStateActors[SlotIndex] = OldMemberActor;
		PartyCharacterIds[SlotIndex] = OldCharacterId;
		NewMemberActor->Destroy();
		return false;
	}

	if (OldMemberActor)
	{
		OldMemberActor->Destroy();
	}

	if (!bReplacingActiveSlot)
	{
		OnPartyChanged.Broadcast();
	}

	return true;
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
	// Prepare appearance before cancellation; missing configuration must not interrupt the current ability.
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
