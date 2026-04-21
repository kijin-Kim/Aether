// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SendGameplayEvent.generated.h"

/**
 * 
 */
UCLASS()
class AETHER_API UAnimNotifyState_SendGameplayEvent : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	
private:
	void SendEventToOwner(USkeletalMeshComponent* MeshComp, const FGameplayTag& Tag);

private:
	UPROPERTY(EditAnywhere, Category = "Aether|GameplayEvent")
	FGameplayTag BeginTag;
	UPROPERTY(EditAnywhere, Category = "Aether|GameplayEvent")
	FGameplayTag EndTag;
};
