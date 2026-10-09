// Fill out your copyright notice in the Description page of Project Settings.

#include "AetherCharacterAssetSubsystem.h"

#include "AbilitySystem/AetherCharacterDefinition.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UAetherCharacterAssetSubsystem::LoadCharacterUI(const FPrimaryAssetId& CharacterId,
                                                     FOnCharacterDefinitionLoaded OnLoaded)
{
	LoadCharacter(CharacterId, {AetherAssetBundles::UI}, MoveTemp(OnLoaded));
}

void UAetherCharacterAssetSubsystem::LoadCharacterDisplay(const FPrimaryAssetId& CharacterId,
                                                          FOnCharacterDefinitionLoaded OnLoaded)
{
	LoadCharacter(CharacterId, {AetherAssetBundles::Display}, MoveTemp(OnLoaded));
}

void UAetherCharacterAssetSubsystem::LoadCharacterGameplay(const FPrimaryAssetId& CharacterId,
                                                           FOnCharacterDefinitionLoaded OnLoaded)
{
	LoadCharacter(CharacterId, {AetherAssetBundles::Gameplay}, MoveTemp(OnLoaded));
}

void UAetherCharacterAssetSubsystem::LoadCharacterGameplayAndDisplay(const FPrimaryAssetId& CharacterId,
                                                                     FOnCharacterDefinitionLoaded OnLoaded)
{
	LoadCharacter(CharacterId, {AetherAssetBundles::Gameplay, AetherAssetBundles::Display}, MoveTemp(OnLoaded));
}

void UAetherCharacterAssetSubsystem::LoadCharacter(const FPrimaryAssetId& CharacterId, TArray<FName> BundleNames,
                                                   FOnCharacterDefinitionLoaded OnLoaded)
{
	if (!CharacterId.IsValid() || BundleNames.IsEmpty())
	{
		OnLoaded.ExecuteIfBound(nullptr);
		return;
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	const uint64 RequestId = ++NextLoadRequestId;
	TSharedPtr<FStreamableHandle> Handle = AssetManager.ChangeBundleStateForPrimaryAssets(
		{CharacterId},
		BundleNames,
		{},
		false,
		FStreamableDelegate::CreateUObject(
			this,
			&UAetherCharacterAssetSubsystem::HandleCharacterLoaded,
			RequestId,
			CharacterId,
			MoveTemp(OnLoaded)));

	if (Handle.IsValid() && !Handle->HasLoadCompleted())
	{
		ActiveLoadHandles.Add(RequestId, MoveTemp(Handle));
	}
}

void UAetherCharacterAssetSubsystem::HandleCharacterLoaded(const uint64 RequestId,
                                                           const FPrimaryAssetId CharacterId,
                                                           FOnCharacterDefinitionLoaded OnLoaded)
{
	ActiveLoadHandles.Remove(RequestId);
	UAetherCharacterDefinition* Definition = GetLoadedCharacterDefinition(CharacterId);
	OnLoaded.ExecuteIfBound(Definition);
}

UAetherCharacterDefinition* UAetherCharacterAssetSubsystem::GetLoadedCharacterDefinition(
	const FPrimaryAssetId& CharacterId) const
{
	return Cast<UAetherCharacterDefinition>(UAssetManager::Get().GetPrimaryAssetObject(CharacterId));
}
