#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Aether/Combat/AetherMeleeTrace.h"
#include "AnimNotifyState_AttackTrace.generated.h"

UCLASS()
class AETHER_API UAnimNotifyState_AttackTrace : public UAnimNotifyState
{
	GENERATED_BODY()
  public:
	virtual void NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float TotalDuration,
	                         const FAnimNotifyEventReference &EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime,
	                        const FAnimNotifyEventReference &EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace") FAetherMeleeTraceSettings Settings;
};
