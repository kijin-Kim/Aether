# 캐릭터 행동 및 파티 시스템

[← README로 돌아가기](../README.md)

---

## ASC Owner, Avatar 분리 구조

오픈월드 파티 태그 시스템에서 오프필드(Off-field) 캐릭터의 독립적인 쿨타임과 에너지 관리를 위해, `PlayerState`에 ASC를 몰아넣지 않고 4개의 보이지 않는 프록시 액터(`APartySlotActor`)를 스폰하여 관리한다.

각 슬롯 액터는 고유의 단일 ASC와 AttributeSet을 가지며(Owner), 활성화된 슬롯의 액터만 실제 조종하는 Pawn(Avatar)과 연결된다.

```cpp
// APartySlotActor.h — 파티 슬롯 하나를 담당하는 프록시 액터 (Owner)
class APartySlotActor : public AInfo, public IAbilitySystemInterface
{
public:
    UPROPERTY()
    UAetherAbilitySystemComponent* ASC;
    
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return ASC; }
};

// AetherCharacter.h — 실제 필드에 스폰되는 Pawn (Avatar)
class AAetherCharacter : public ACharacter, public IAbilitySystemInterface
{
public:
    // 슬롯 교체 시 주입받은 현재 활성 ASC 포인터 반환
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return CachedASC; }

    UPROPERTY()
    UAetherAbilitySystemComponent* CachedASC; 
};

// AetherPlayerState.h — 파티 슬롯 액터들을 관리하고 교체를 지휘
void AAetherPlayerState::SwitchToSlot(int32 NewSlotIndex)
{
    // 1. 기존 슬롯의 ASC와 현재 Pawn(Avatar)의 연결 해제
    PartySlots[ActiveSlotIndex]->ASC->InitAbilityActorInfo(PartySlots[ActiveSlotIndex], nullptr);

    ActiveSlotIndex = NewSlotIndex;
    APartySlotActor* NewSlotActor = PartySlots[NewSlotIndex];

    // 2. 새 Pawn 스폰 또는 활성화
    AAetherCharacter* NewPawn = SpawnOrActivateCharacterForSlot(NewSlotIndex);

    // 3. 새 슬롯의 ASC(Owner)와 Pawn(Avatar) 연결
    NewSlotActor->ASC->InitAbilityActorInfo(NewSlotActor, NewPawn);
    NewPawn->CachedASC = NewSlotActor->ASC;

    GetPlayerController()->Possess(NewPawn);
}
```

---

## 전투 & 이동 행동 목록

| 어빌리티 | 활성화 조건 | 구현 | 종료 처리 |
|----------|------------|-----------|----------|
| **기본 공격 (콤보)** | 태그 조건 없음 | AbilityTask `PlayMontageAndWait` + `WaitGameplayEvent`. `Combat.Hit.Normal` 수신 시 히트박스 활성화. 연속 입력으로 다음 콤보 GA 자동 활성화 | 마지막 콤보 완료 또는 취소. 원소 부착 GE 자동 적용 |
| **원소 스킬 (E)** | 쿨다운 GE 없음 + `Skill.Available` | InputPressed/Released Task로 탭·홀드 분기. 쿨타임·에너지 획득 GE 자동 연계 | 효과 완료 또는 강제 중단. CooldownTag로 UI 비활성 |
| **원소 폭발 (Q)** | 에너지 Cost GE 충족 + `Burst.Ready` | Cost GE 에너지 차감. 폭발 중 `Status.Invincible` 부여. | 연출 완료 후 EndAbility. Invincible 태그 제거 |
| **등반 (Climbing)** | 등반 가능 표면 + 스태미나 > 0 |  | 스태미나 소진 또는 입력 해제. 낙하 GA 연계 |
| **수영 (Swimming)** | Water Volume Overlap + 스태미나 > 0 |  | Water Volume Exit 또는 스태미나 소진. 익사 GA 연계 |
| **활공 (Gliding)** | 공중 + 스태미나 > 0 | 상승 기류 발생시 근접 시 CMC에 상향 Force 지속 적용 | 스태미나 소진 또는 착지 |

---