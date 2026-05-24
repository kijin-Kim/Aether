// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_AttackTrace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Aether/AetherGameplayTags.h"
#include "Aether/Weapon/WeaponHolder.h"

UAnimNotifyState_AttackTrace::UAnimNotifyState_AttackTrace()
	: StartSocketName("weapon_start")
	  , EndSocketName("weapon_end")
	  , TraceChannel(ECC_Pawn)
	  , HitEventTag(AetherGameplayTags::Event_Montage_AttackHit)
	  , bDrawDebug(false)

{
}

void UAnimNotifyState_AttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!MeshComp || !MeshComp->GetOwner()) return;

	FTraceState& State = StateMap.FindOrAdd(MeshComp);
	State.AlreadyHitActors.Empty();
	State.bInitialized = false;

	if (bDrawDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("[AttackTrace] Begin - Owner: %s"),
		       *MeshComp->GetOwner()->GetName());
	}
}

void UAnimNotifyState_AttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (!MeshComp || !MeshComp->GetOwner()) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	IWeaponHolder* WeaponHolder = Cast<IWeaponHolder>(OwnerActor);
	if (!WeaponHolder)
	{
		return;
	}

	UStaticMeshComponent* WeaponMesh = WeaponHolder->GetWeaponMesh();
	if (!WeaponMesh)
	{
		return;
	}

	FTraceState& State = StateMap.FindOrAdd(MeshComp);

	const FVector CurStart = WeaponMesh->GetSocketLocation(StartSocketName);
	const FVector CurEnd = WeaponMesh->GetSocketLocation(EndSocketName);

	// 첫 Tick: 이전 위치를 현재 위치로 초기화 (순간이동 방지)
	if (!State.bInitialized)
	{
		State.PrevStart = CurStart;
		State.PrevEnd = CurEnd;
		State.bInitialized = true;
		return;
	}

	const FVector PrevMid = (State.PrevStart + State.PrevEnd) * 0.5f;
	const FVector CurMid = (CurStart + CurEnd) * 0.5f;

	// 현재 Capsule의 Z up축을 무기 축으로 맞춤
	const FVector WeaponDir = (CurEnd - CurStart).GetSafeNormal();
	const FQuat CapsuleRot = FRotationMatrix::MakeFromZ(WeaponDir).ToQuat();

	const float HalfHeight = (CurEnd - CurStart).Size() * 0.5f;

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AttackTrace), false, OwnerActor);
	Params.AddIgnoredActor(OwnerActor);

	OwnerActor->GetWorld()->SweepMultiByChannel(
		Hits,
		PrevMid,
		CurMid,
		CapsuleRot,
		TraceChannel,
		FCollisionShape::MakeCapsule(20.0f, HalfHeight),
		Params
	);


	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor) continue;

		if (State.AlreadyHitActors.Contains(HitActor)) continue;
		State.AlreadyHitActors.Add(HitActor);

		FGameplayEventData EventData;
		EventData.EventTag = HitEventTag;
		EventData.Instigator = OwnerActor;
		EventData.Target = HitActor;
		EventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			OwnerActor, HitEventTag, EventData
		);

		if (bDrawDebug)
		{
			DrawDebugSphere(OwnerActor->GetWorld(), Hit.ImpactPoint, 10.f, 12, FColor::Green, false, 1.0f);
		}
	}

	if (bDrawDebug)
	{
		DrawDebugCapsule(OwnerActor->GetWorld(),
		                 CurMid, HalfHeight, 20.0f, CapsuleRot,
		                 FColor::Red, false, 1.0f);
	}

	State.PrevStart = CurStart;
	State.PrevEnd = CurEnd;
}

void UAnimNotifyState_AttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	StateMap.Remove(MeshComp);

	if (bDrawDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("[AttackTrace] End"));
	}
}
