#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "AetherAttackTestTarget.generated.h"
class UAetherAbilitySystemComponent;
class UAetherBaseAttributeSet;
class UCapsuleComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

// Placeable verification target; no enemy AI, hit reaction or death behavior.
UCLASS()
class AETHER_API AAetherAttackTestTarget : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
  public:
	AAetherAttackTestTarget();
	virtual UAbilitySystemComponent *GetAbilitySystemComponent() const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UCapsuleComponent> Collision;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Visual;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> HealthLabel;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UAetherAbilitySystemComponent> ASC;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UAetherBaseAttributeSet> Attributes;
};
