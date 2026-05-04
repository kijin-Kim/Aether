// Fill out your copyright notice in the Description page of Project Settings.


#include "AetherCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Aether/AbilitySystem/AetherAbilitySet.h"
#include "Aether/AbilitySystem/AttributeSet/AetherBaseAttributeSet.h"
#include "Aether/AetherGameplayTags.h"
#include "Aether/Input/AetherInputComponent.h"
#include "EnhancedInputComponent.h"
#include "Aether/Aether.h"
#include "Aether/AetherCharacterDatabase.h"
#include "Aether/AbilitySystem/AetherCharacterData.h"
#include "Aether/AbilitySystem/AttributeSet/AetherHeroAttributeSet.h"
#include "Aether/AbilitySystem/Effects/AetherGameplayEffect_ElementalDamage.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"


AAetherCharacter::AAetherCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AetherASC = CreateDefaultSubobject<UAetherAbilitySystemComponent>("AetherASC");

	bUseControllerRotationYaw = false;

	GetCapsuleComponent()->SetCapsuleHalfHeight(90.0f);
	GetCapsuleComponent()->SetCapsuleRadius(35.0f);

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -89.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));


	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());
	SpringArmComponent->TargetArmLength = 400.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraComponent->SetupAttachment(SpringArmComponent);

	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AAetherCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UAetherCharacterData* CharacterData = GetGameInstance()->GetSubsystem<UAetherCharacterDatabase>()->GetCharacterByID(CharacterId))
	{
		if (UAetherInputConfig* InputConfig = CharacterData->InputConfig.LoadSynchronous())
		{
			if (UAetherInputComponent* AetherInputComponent = CastChecked<UAetherInputComponent>(PlayerInputComponent))
			{
				AetherInputComponent->BindAbilityInputAction(InputConfig, this, &AAetherCharacter::AbilityInputPressed, &AAetherCharacter::AbilityInputReleased);
				AetherInputComponent->BindInputAction(InputConfig, AetherGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &AAetherCharacter::Move);
				AetherInputComponent->BindInputAction(InputConfig, AetherGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &AAetherCharacter::Look);
			}
		}
	}
}

void AAetherCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SetOnField(true);
	AetherASC->InitAbilityActorInfo(this, this);
}

void AAetherCharacter::UnPossessed()
{
	Super::UnPossessed();
	SetOnField(false);
	AetherASC->ClearInputs();
}

void AAetherCharacter::ReceiveElementalAttack_Implementation(AActor* SourceActor, FGameplayTag ElementTypeTag, float Damage, float Gauge)
{
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);
	UAbilitySystemComponent* TargetASC = GetAbilitySystemComponent();
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(SourceASC->GetAvatarActor());
	Context.AddInstigator(SourceASC->GetAvatarActor(), SourceASC->GetAvatarActor());

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(UAetherGameplayEffect_ElementalDamage::StaticClass(), 1.0f, Context);
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	
	Spec->SetSetByCallerMagnitude(AetherGameplayTags::Data_Damage, Damage);
	Spec->SetSetByCallerMagnitude(AetherGameplayTags::Data_AuraGauge, Gauge);
	Spec->AddDynamicAssetTag(ElementTypeTag);

	FActiveGameplayEffectHandle AppliedHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*Spec, TargetASC);
}


void AAetherCharacter::InitializeFromCharacterData(FName NewCharacterId)
{
	CharacterId = NewCharacterId;
	if (UAetherCharacterData* CharacterData = GetGameInstance()->GetSubsystem<UAetherCharacterDatabase>()->GetCharacterByID(CharacterId))
	{
		for (const TSoftObjectPtr<UAetherAbilitySet>& AbilitySetPtr : CharacterData->AbilitySets)
		{
			if (UAetherAbilitySet* AbilitySet = AbilitySetPtr.LoadSynchronous())
			{
				if (AbilitySet)
				{
					AbilitySet->InitializeAbilitySystem(AetherASC, GrantedAbilitySetHandles, nullptr);
				}
			}
		}

		if (CharacterData->Mesh)
		{
			GetMesh()->SetSkeletalMesh(CharacterData->Mesh.LoadSynchronous());
		}

		if (CharacterData->AnimInstanceClass)
		{
			GetMesh()->SetAnimInstanceClass(CharacterData->AnimInstanceClass.LoadSynchronous());
		}
	}
}

void AAetherCharacter::SetOnField(bool bSetOnField)
{
	bOnField = bSetOnField;
	// TODO: 다른 것들도 꺼서 최적화
	SetActorHiddenInGame(!bOnField);
	SetActorTickEnabled(bOnField);
	SetActorEnableCollision(bOnField);

}

void AAetherCharacter::AbilityInputPressed(FGameplayTag InputTag)
{
	AetherASC->AbilityInputPressed(InputTag);
}

void AAetherCharacter::AbilityInputReleased(FGameplayTag InputTag)
{
	AetherASC->AbilityInputReleased(InputTag);
}

void AAetherCharacter::Move(const FInputActionValue& InputActionValue)
{
	FVector2D InputVector = InputActionValue.Get<FVector2D>();
	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(ForwardDirection, InputVector.Y);
	AddMovementInput(RightDirection, InputVector.X);
}

void AAetherCharacter::Look(const FInputActionValue& InputActionValue)
{
	FVector2D InputVector = InputActionValue.Get<FVector2D>();
	AddControllerPitchInput(InputVector.Y);
	AddControllerYawInput(InputVector.X);
}
