#include "AnimNotifyState_AttackTrace.h"
#include "Animation/ActiveMontageInstanceScope.h"
#include "Animation/AnimNotifyQueue.h"
#include "Components/SkeletalMeshComponent.h"
#include "Aether/PartySystem/AetherFieldCharacterPawn.h"

namespace
{
AAetherFieldCharacterPawn *Resolve(USkeletalMeshComponent *Mesh, const FAnimNotifyEventReference &Event, FAetherTraceKey &Key)
{
	if (!Mesh)
		return nullptr;
	const auto *Context = Event.GetContextData<UE::Anim::FAnimNotifyMontageInstanceContext>();
	if (!Context || !Event.GetNotify())
		return nullptr;
	auto *Pawn = Cast<AAetherFieldCharacterPawn>(Mesh->GetOwner());
	if (!Pawn || Pawn->GetMesh() != Mesh)
		return nullptr;
	Key = {Context->MontageInstanceID, Event.GetNotify()};
	return Pawn;
}
} // namespace
void UAnimNotifyState_AttackTrace::NotifyBegin(USkeletalMeshComponent *Mesh, UAnimSequenceBase *Animation, float Duration,
                                               const FAnimNotifyEventReference &Event)
{
	Super::NotifyBegin(Mesh, Animation, Duration, Event);
	FAetherTraceKey Key;
	if (auto *Pawn = Resolve(Mesh, Event, Key))
		Pawn->BeginAttackTrace(Key, Settings);
}
void UAnimNotifyState_AttackTrace::NotifyTick(USkeletalMeshComponent *Mesh, UAnimSequenceBase *Animation, float Delta, const FAnimNotifyEventReference &Event)
{
	Super::NotifyTick(Mesh, Animation, Delta, Event);
	FAetherTraceKey Key;
	if (auto *Pawn = Resolve(Mesh, Event, Key))
		Pawn->UpdateAttackTrace(Key);
}
void UAnimNotifyState_AttackTrace::NotifyEnd(USkeletalMeshComponent *Mesh, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &Event)
{
	Super::NotifyEnd(Mesh, Animation, Event);
	FAetherTraceKey Key;
	if (auto *Pawn = Resolve(Mesh, Event, Key))
		Pawn->EndAttackTrace(Key);
}
