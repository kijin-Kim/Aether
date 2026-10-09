// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_AttackTrace.generated.h"

/**
 * 
 */
UCLASS()
class AETHER_API UAnimNotifyState_AttackTrace : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UAnimNotifyState_AttackTrace();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	
private:
	UPROPERTY(EditAnywhere, Category="Trace")
	FName StartSocketName;

	UPROPERTY(EditAnywhere, Category="Trace")
	FName EndSocketName;

	UPROPERTY(EditAnywhere, Category="Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel;

	UPROPERTY(EditAnywhere, Category="Trace", meta=(ClampMin="1.0"))
	float TraceRadius;

	UPROPERTY(EditAnywhere, Category="Trace")
	FGameplayTag HitEventTag;
	
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDrawDebug;
	
	
	struct FTraceState
	{
		TSet<TWeakObjectPtr<AActor>> AlreadyHitActors;
		FVector PrevStart;
		FVector PrevEnd;
		bool bInitialized = false;
	};

	// AnimNotifyState는 공유 인스턴스라 MeshComp마다 상태 관리해야함
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FTraceState> StateMap;
};
