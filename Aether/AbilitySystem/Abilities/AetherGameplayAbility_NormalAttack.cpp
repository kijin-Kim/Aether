#include "AetherGameplayAbility_NormalAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Aether/PartySystem/AetherFieldCharacterPawn.h"
#include "Aether/Aether.h"

void UAetherGameplayAbility_NormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                                          const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData *TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	AAetherFieldCharacterPawn *Pawn = ActorInfo ? Cast<AAetherFieldCharacterPawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!Pawn || !Pawn->GetMesh() || !Pawn->GetMesh()->GetAnimInstance() || GroundAttackAnimations.IsEmpty() || !GroundAttackAnimations[0] ||
	    !FMath::IsFinite(BaseDamage) || BaseDamage < 0.f || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}
	TracePawn = Pawn;
	TraceRegistration = Pawn->RegisterAttackReceiver(this);
	if (!TraceRegistration)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}
	UAbilityTask_PlayMontageAndWait *Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, GroundAttackAnimations[0]);
	Task->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	Task->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	Task->OnCancelled.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	Task->ReadyForActivation();
	if (!IsActive())
		return;
	FAnimMontageInstance *Instance = Pawn->GetMesh()->GetAnimInstance()->GetActiveInstanceForMontage(GroundAttackAnimations[0]);
	if (!Instance)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}
	MontageInstanceId = Instance->GetInstanceID();
}

bool UAetherGameplayAbility_NormalAttack::Accepts(uint64 Registration, const FAetherTraceKey &Key) const
{
	return IsActive() && TracePawn.IsValid() && TraceRegistration != 0 && TraceRegistration == Registration && MontageInstanceId != INDEX_NONE &&
	       Key.MontageInstance == MontageInstanceId && Key.Window;
}

void UAetherGameplayAbility_NormalAttack::BeginTrace(uint64 Registration, const FAetherTraceKey &Key, const FAetherMeleeTraceSettings &Settings)
{
	if (!Accepts(Registration, Key) || TraceSessions.Contains(Key))
		return;
	FAetherTraceSession Session;
	Session.Settings = Settings;
	if (!AetherMeleeTrace::ResolveSegment(TracePawn->GetMesh(), Settings, Session.PreviousStart, Session.PreviousEnd))
	{
		UE_LOG(LogAether, Warning, TEXT("NormalAttack: invalid trace socket/settings: %s"), *Settings.ReferenceSocket.ToString());
		return;
	}
	TraceSessions.Add(Key, MoveTemp(Session));
	UpdateTrace(Registration, Key);
}

void UAetherGameplayAbility_NormalAttack::UpdateTrace(uint64 Registration, const FAetherTraceKey &Key)
{
	if (!Accepts(Registration, Key))
		return;
	FAetherTraceSession *Session = TraceSessions.Find(Key);
	if (!Session)
		return;
	FVector Start, End;
	if (!AetherMeleeTrace::ResolveSegment(TracePawn->GetMesh(), Session->Settings, Start, End))
	{
		TraceSessions.Remove(Key);
		return;
	}
	TArray<FHitResult> Hits;
	AetherMeleeTrace::Sweep(GetWorld(), TracePawn.Get(), Session->PreviousStart, Session->PreviousEnd, Start, End, Session->Settings.Radius,
	                        Session->Settings.bDrawDebug, Hits);
	Session->PreviousStart = Start;
	Session->PreviousEnd = End;
	for (const FHitResult &Hit : Hits)
	{
		// GE callbacks can end the ability; reacquire state after every application.
		if (!Accepts(Registration, Key))
			break;
		Session = TraceSessions.Find(Key);
		if (!Session)
			break;
		AActor *Target = Hit.GetActor();
		if (!IsValid(Target) || Session->HitActors.Contains(Target))
			continue;
		Session->HitActors.Add(Target);
		ApplyElementalAttackToTarget(UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit), DamageElement, BaseDamage, AuraGauge);
	}
}
void UAetherGameplayAbility_NormalAttack::EndTrace(uint64 Registration, const FAetherTraceKey &Key)
{
	if (Accepts(Registration, Key))
		TraceSessions.Remove(Key);
}
void UAetherGameplayAbility_NormalAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                                     const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Invalidate before montage shutdown can emit more notifies.
	const uint64 Registration = TraceRegistration;
	TraceRegistration = 0;
	MontageInstanceId = INDEX_NONE;
	TraceSessions.Reset();
	if (TracePawn.IsValid())
		TracePawn->UnregisterAttackReceiver(this, Registration);
	TracePawn.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
void UAetherGameplayAbility_NormalAttack::OnMontageCompleted()
{
	if (IsActive())
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
void UAetherGameplayAbility_NormalAttack::OnMontageInterrupted()
{
	if (IsActive())
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}
