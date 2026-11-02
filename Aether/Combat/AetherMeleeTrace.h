#pragma once

#include "CoreMinimal.h"
#include "AetherMeleeTrace.generated.h"

class USkeletalMeshComponent;
struct FAnimNotifyEvent;

// Authored on the animation, independent of the equipped weapon mesh.
USTRUCT(BlueprintType)
struct FAetherMeleeTraceSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ReferenceSocket = TEXT("hand_r");
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector LocalStart = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector LocalEnd = FVector(80.f, 0.f, 0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0")) float Radius = 15.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDrawDebug = false;
};

struct FAetherTraceKey
{
	int32 MontageInstance = INDEX_NONE;
	const FAnimNotifyEvent *Window = nullptr;
	bool operator==(const FAetherTraceKey &Other) const
	{
		return MontageInstance == Other.MontageInstance && Window == Other.Window;
	}
	friend uint32 GetTypeHash(const FAetherTraceKey &Key)
	{
		return HashCombine(GetTypeHash(Key.MontageInstance), PointerHash(Key.Window));
	}
};

struct FAetherTraceSession
{
	FAetherMeleeTraceSettings Settings;
	FVector PreviousStart = FVector::ZeroVector;
	FVector PreviousEnd = FVector::ZeroVector;
	TSet<TWeakObjectPtr<AActor>> HitActors;
};

namespace AetherMeleeTrace
{
bool ResolveSegment(const USkeletalMeshComponent *Mesh, const FAetherMeleeTraceSettings &Settings, FVector &Start, FVector &End);
void Sweep(UWorld *World, AActor *IgnoredActor, const FVector &PreviousStart, const FVector &PreviousEnd, const FVector &Start, const FVector &End,
           float Radius, bool bDrawDebug, TArray<FHitResult> &Hits);
} // namespace AetherMeleeTrace
