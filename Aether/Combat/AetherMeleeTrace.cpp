#include "AetherMeleeTrace.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

bool AetherMeleeTrace::ResolveSegment(const USkeletalMeshComponent *Mesh, const FAetherMeleeTraceSettings &Settings, FVector &Start, FVector &End)
{
	if (!Mesh || Settings.ReferenceSocket.IsNone() || !Mesh->DoesSocketExist(Settings.ReferenceSocket) || !FMath::IsFinite(Settings.Radius) ||
	    Settings.Radius < 1.f || Settings.LocalStart.ContainsNaN() || Settings.LocalEnd.ContainsNaN())
		return false;
	const FTransform Reference = Mesh->GetSocketTransform(Settings.ReferenceSocket);
	// Character/weapon visual scale must not change authored reach.
	Start = Reference.TransformPositionNoScale(Settings.LocalStart);
	End = Reference.TransformPositionNoScale(Settings.LocalEnd);
	return !Start.ContainsNaN() && !End.ContainsNaN();
}

void AetherMeleeTrace::Sweep(UWorld *World, AActor *IgnoredActor, const FVector &PreviousStart, const FVector &PreviousEnd, const FVector &Start,
                             const FVector &End, float Radius, bool bDrawDebug, TArray<FHitResult> &Hits)
{
	Hits.Reset();
	if (!World || !FMath::IsFinite(Radius) || Radius < 1.f)
		return;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AetherMeleeTrace), false, IgnoredActor);
	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_Pawn);
	const float Travel = FMath::Max(FVector::Distance(PreviousStart, Start), FVector::Distance(PreviousEnd, End));
	const int32 Steps = FMath::Clamp(FMath::CeilToInt(Travel / (Radius * 0.5f)), 1, 64);
	FVector PreviousMid = (PreviousStart + PreviousEnd) * 0.5f;
	for (int32 Step = 0; Step <= Steps; ++Step)
	{
		const float Alpha = static_cast<float>(Step) / Steps;
		const FVector A = FMath::Lerp(PreviousStart, Start, Alpha);
		const FVector B = FMath::Lerp(PreviousEnd, End, Alpha);
		const FVector Mid = (A + B) * 0.5f;
		const FVector Axis = B - A;
		const FQuat Rotation = Axis.IsNearlyZero() ? FQuat::Identity : FRotationMatrix::MakeFromZ(Axis).ToQuat();
		const float HalfHeight = Axis.Size() * 0.5f + Radius;
		TArray<FHitResult> SampleHits;
		// Object query collects multiple blocking pawns instead of stopping at the first blocking target.
		World->SweepMultiByObjectType(SampleHits, PreviousMid, Mid, Rotation, Objects, FCollisionShape::MakeCapsule(Radius, HalfHeight), Params);
		Hits.Append(SampleHits);
		if (bDrawDebug)
			DrawDebugCapsule(World, Mid, HalfHeight, Radius, Rotation, FColor::Green, false, 0.05f);
		PreviousMid = Mid;
	}
}
