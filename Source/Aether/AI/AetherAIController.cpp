// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherAIController.h"

#include "Aether/Aether.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/Character/AetherCharacterBase.h"
#include "Components/StateTreeAIComponent.h"

AAetherAIController::AAetherAIController()
{
	PrimaryActorTick.bCanEverTick = false;

	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>("StateTreeAI");
	StateTreeComponent->SetStartLogicAutomatically(true);
}

void AAetherAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AAetherCharacterBase* CharacterBase = Cast<AAetherCharacterBase>(InPawn))
	{
		CachedASC = CharacterBase->GetAetherAbilitySystemComponent();
	}
	else
	{
		UE_LOG(LogAether, Warning, TEXT("AAetherAIController possessed non-AetherCharacterBase pawn: %s"), *GetNameSafe(InPawn));
	}
}

void AAetherAIController::OnUnPossess()
{
	CachedASC = nullptr;
	Super::OnUnPossess();
}

void AAetherAIController::TryActivateAbilityByTag(FGameplayTag InputTag)
{
	if (!CachedASC || !InputTag.IsValid())
	{
		return;
	}
	CachedASC->AbilityInputPressed(InputTag);
	CachedASC->ProcessInputs();
}
