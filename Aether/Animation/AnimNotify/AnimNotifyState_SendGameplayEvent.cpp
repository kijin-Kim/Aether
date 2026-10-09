// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_SendGameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UAnimNotifyState_SendGameplayEvent::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (BeginTag.IsValid())
	{
		SendEventToOwner(MeshComp, BeginTag);
	}
}

void UAnimNotifyState_SendGameplayEvent::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (EndTag.IsValid())
	{
		SendEventToOwner(MeshComp, EndTag);
	}
}

void UAnimNotifyState_SendGameplayEvent::SendEventToOwner(USkeletalMeshComponent* MeshComp, const FGameplayTag& Tag)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner))
		{
			FGameplayEventData Data;
			Data.EventTag = Tag;
			Data.Instigator = Owner;
			ASC->HandleGameplayEvent(Tag, &Data);
		}
	}
}
