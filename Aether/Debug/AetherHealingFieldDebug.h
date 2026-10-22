#pragma once
#include "CoreMinimal.h"
#if WITH_EDITOR
class UWorld;
class AAetherHealingFieldActor;
namespace AetherHealingFieldDebug
{
bool Execute(UWorld* World, const TArray<FString>& Args);
void Report(const AAetherHealingFieldActor* Field, const TCHAR* Event);
} // namespace AetherHealingFieldDebug
#endif
