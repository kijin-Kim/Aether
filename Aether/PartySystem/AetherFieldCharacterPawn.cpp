// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherFieldCharacterPawn.h"
#include "Aether/AbilitySystem/Abilities/AetherGameplayAbility_NormalAttack.h"

#include "Aether/Input/AetherInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"
#include "GameFramework/SpringArmComponent.h"


AAetherFieldCharacterPawn::AAetherFieldCharacterPawn()
{
	bReplicates = false;
	SetReplicateMovement(false);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());
	SpringArmComponent->TargetArmLength = 400.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraComponent->SetupAttachment(SpringArmComponent);

	FieldStreamingSourceComponent =
		CreateDefaultSubobject<UWorldPartitionStreamingSourceComponent>("FieldStreamingSource");
	FieldStreamingSourceComponent->DisableStreamingSource();

	OverrideInputComponentClass = UAetherInputComponent::StaticClass();
}

void AAetherFieldCharacterPawn::SetFieldStreamingSourceEnabled(const bool bEnabled)
{
	if (bEnabled)
	{
		FieldStreamingSourceComponent->EnableStreamingSource();
	}
	else
	{
		FieldStreamingSourceComponent->DisableStreamingSource();
	}
}

bool AAetherFieldCharacterPawn::ApplyCharacterAppearance(
	USkeletalMesh* SkeletalMesh, TSubclassOf<UAnimInstance> AnimInstanceClass)
{
	if (!GetMesh() || !SkeletalMesh)
	{
		return false;
	}

	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetSkeletalMesh(SkeletalMesh);
	GetMesh()->SetAnimInstanceClass(AnimInstanceClass);
	return true;
}

uint64 AAetherFieldCharacterPawn::RegisterAttackReceiver(UAetherGameplayAbility_NormalAttack* Receiver)
{
	if (!IsValid(Receiver) || AttackReceiver.IsValid()) return 0;
	AttackReceiver = Receiver;
	return ++AttackRegistration;
}

void AAetherFieldCharacterPawn::UnregisterAttackReceiver(UAetherGameplayAbility_NormalAttack* Receiver, uint64 Registration)
{
	if (AttackReceiver.Get() == Receiver && AttackRegistration == Registration) AttackReceiver.Reset();
}

void AAetherFieldCharacterPawn::BeginAttackTrace(const FAetherTraceKey& Key, const FAetherMeleeTraceSettings& Settings)
{
	if (AttackReceiver.IsValid()) AttackReceiver->BeginTrace(AttackRegistration, Key, Settings);
}

void AAetherFieldCharacterPawn::UpdateAttackTrace(const FAetherTraceKey& Key)
{
	if (AttackReceiver.IsValid()) AttackReceiver->UpdateTrace(AttackRegistration, Key);
}

void AAetherFieldCharacterPawn::EndAttackTrace(const FAetherTraceKey& Key)
{
	if (AttackReceiver.IsValid()) AttackReceiver->EndTrace(AttackRegistration, Key);
}
