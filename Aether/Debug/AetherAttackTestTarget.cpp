#include "AetherAttackTestTarget.h"
#include "Aether/AbilitySystem/AetherAbilitySystemComponent.h"
#include "Aether/AbilitySystem/AttributeSet/AetherBaseAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "UObject/ConstructorHelpers.h"

AAetherAttackTestTarget::AAetherAttackTestTarget()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
	Collision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("TargetCollision"));
	SetRootComponent(Collision);
	Collision->InitCapsuleSize(30.f, 80.f);
	Collision->SetCollisionProfileName(TEXT("Pawn"));
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetVisual"));
	Visual->SetupAttachment(Collision);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	Visual->SetStaticMesh(Cube.Object);
	Visual->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.5f));
	HealthLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Health"));
	HealthLabel->SetupAttachment(Collision);
	HealthLabel->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
	HealthLabel->SetWorldSize(20.f);
	ASC = CreateDefaultSubobject<UAetherAbilitySystemComponent>(TEXT("ASC"));
	Attributes = CreateDefaultSubobject<UAetherBaseAttributeSet>(TEXT("Attributes"));
}

UAbilitySystemComponent *AAetherAttackTestTarget::GetAbilitySystemComponent() const
{
	return ASC;
}
void AAetherAttackTestTarget::BeginPlay()
{
	Super::BeginPlay();
	ASC->InitAbilityActorInfo(this, this);
	ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetMaxHealthAttribute(), 1000.f);
	ASC->SetNumericAttributeBase(UAetherBaseAttributeSet::GetHealthAttribute(), 1000.f);
}
void AAetherAttackTestTarget::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	HealthLabel->SetText(FText::FromString(FString::Printf(TEXT("HP %.0f"), Attributes->GetHealth())));
}
