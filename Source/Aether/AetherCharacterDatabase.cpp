// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherCharacterDatabase.h"

#include "Aether.h"
#include "AbilitySystem/AetherCharacterData.h"
#include "Engine/AssetManager.h"

void UAetherCharacterDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UAssetManager& AssetManager = UAssetManager::Get();

	// "Character" 타입으로 등록된 모든 프라이머리 에셋(Primary Asset)의 ID 목록을 가져옵니다.
	TArray<FPrimaryAssetId> CharacterAssetIds;
	AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("Character"), CharacterAssetIds);
	UE_LOG(LogAether, Log, TEXT("Found %d Character Assets"), CharacterAssetIds.Num());


	// 가져온 캐릭터 ID 목록을 바탕으로 에셋들을 비동기적으로 로드합니다.
	TArray<FName> EmptyBundles;
	TSharedPtr<FStreamableHandle> Handle = AssetManager.LoadPrimaryAssets(CharacterAssetIds, EmptyBundles,
	                                                                      FStreamableDelegate::CreateUObject(this, &UAetherCharacterDatabase::OnCharacterIndexLoaded));
	
	// 로드 작업이 유효하다면, 핸들을 ActiveLoadHandles 배열에 저장하여 
	// 메모리에서 해제(GC)되거나 예기치 않게 로드가 취소되는 것을 방지합니다.
	if (Handle.IsValid())
		ActiveLoadHandles.Add(Handle);
}

void UAetherCharacterDatabase::OnCharacterIndexLoaded()
{
	UAssetManager& AssetManager = UAssetManager::Get();

	// "Character" 타입으로 등록된 모든 프라이머리 에셋(Primary Asset)의 ID 목록을 가져옵니다.
	TArray<FPrimaryAssetId> CharacterAssetIds;
	AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("Character"), CharacterAssetIds);
	UE_LOG(LogAether, Log, TEXT("Found %d Character Assets"), CharacterAssetIds.Num());


	for (const FPrimaryAssetId& CharacterId : CharacterAssetIds)
	{
		if (UAetherCharacterData* CharacterData = Cast<UAetherCharacterData>(AssetManager.GetPrimaryAssetObject(CharacterId)))
		{
			LoadedCharacters.Add(CharacterData->CharacterId, CharacterData);
			UE_LOG(LogAether, Log, TEXT("Loaded Character Data: %s"), *CharacterData->GetName());
		}
		else
		{
			UE_LOG(LogAether, Warning, TEXT("Failed to load Character Data for Asset ID: %s"), *CharacterId.ToString());
		}
	}
}


UAetherCharacterData* UAetherCharacterDatabase::GetCharacterByID(FName CharacterId)
{
	// 1. 이미 캐싱되어 있는지 확인
	if (TObjectPtr<UAetherCharacterData>* Found = LoadedCharacters.Find(CharacterId))
		return *Found;

	UAssetManager& AM = UAssetManager::Get();
	FPrimaryAssetId Id("Character", CharacterId);

	// 2. 프라이머리 에셋 ID를 기반으로 실제 에셋 경로(SoftObjectPath)를 가져옴
	FSoftObjectPath AssetPath = AM.GetPrimaryAssetPath(Id);

	if (!AssetPath.IsValid())
	{
		UE_LOG(LogAether, Warning, TEXT("Failed to find path for Character ID: %s"), *CharacterId.ToString());
		return nullptr;
	}

	// 3. 스트리머블 매니저의 LoadSynchronous를 사용하여 동기식 로드 후 캐스팅
	UAetherCharacterData* Data = Cast<UAetherCharacterData>(
		AM.GetStreamableManager().LoadSynchronous(AssetPath)
	);

	// 4. 로드 성공 시 맵에 캐싱
	if (Data)
	{
		LoadedCharacters.Add(CharacterId, Data);
	}

	return Data;
}


void UAetherCharacterDatabase::LoadCharacterDataAsync(const TArray<FName>& CharacterIds, FStreamableDelegate OnAllLoaded)
{
	// 로드해야 할 무거운 에셋(메시, 애니메이션 등)의 경로를 담을 배열입니다.
	TArray<FSoftObjectPath> PathsToLoad;

	// 파티에 포함된 캐릭터 ID들을 순회합니다.
	for (FName Id : CharacterIds)
	{
		// 먼저 캐릭터 데이터 에셋(메타데이터)을 가져옵니다.
		UAetherCharacterData* Data = GetCharacterByID(Id);
		if (!Data) continue; // 데이터를 찾지 못하면 다음 캐릭터로 넘어갑니다.

		// 헤비 자산들 추가: TSoftObjectPtr 등으로 선언된 무거운 에셋들의 경로를 추출합니다.
		// IsNull()을 체크하여 설정되어 있는 에셋만 로드 대상에 추가합니다.
		if (!Data->Mesh.IsNull())
			PathsToLoad.Add(Data->Mesh.ToSoftObjectPath());

		if (!Data->AnimInstanceClass.IsNull())
			PathsToLoad.Add(Data->AnimInstanceClass.ToSoftObjectPath());

		if (!Data->CharacterClass.IsNull())
			PathsToLoad.Add(Data->CharacterClass.ToSoftObjectPath());
	}

	// 로드할 에셋이 하나도 없다면(이미 다 로드되었거나 빈 파티인 경우) 
	// 즉시 콜백 함수를 실행하고 종료합니다.
	if (PathsToLoad.Num() == 0)
	{
		OnAllLoaded.ExecuteIfBound();
		return;
	}

	// 스트리머블 매니저를 통해 수집한 무거운 에셋들의 비동기 로드를 요청합니다.
	FStreamableManager& Streamable = UAssetManager::Get().GetStreamableManager();

	// 로드가 모두 완료되면 매개변수로 받은 OnAllLoaded 델리게이트가 실행됩니다.
	TSharedPtr<FStreamableHandle> Handle =
		Streamable.RequestAsyncLoad(PathsToLoad, OnAllLoaded);

	// // 진행 중인 로드 작업이 취소되지 않도록 핸들을 보관합니다.
	if (Handle.IsValid())
		ActiveLoadHandles.Add(Handle);
}
