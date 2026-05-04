// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherHUD.h"

#include "EngineUtils.h"
#include "Aether/Element/ElementalReactiveInterface.h"

void AAetherHUD::GetDebugActorList(TArray<AActor*>& InOutList)
{
	Super::GetDebugActorList(InOutList);

	UWorld* World = GetWorld();

	static const FName ElementDebugName(TEXT("Element"));

	// If we're viewing animations, add all actors using an AnimInstance.
	if (ShouldDisplayDebug(ElementDebugName))
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (It->Implements<UElementalReactiveInterface>())
			{
				AddActorToDebugList(*It, InOutList, World);
			}
		}
	}
}
