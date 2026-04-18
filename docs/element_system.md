# 원소 시스템

[← README로 돌아가기](../README.md)

---

## 원소 시스템

피격 Actor에 `Element.Aura.Pyro` 같은 태그가 부착된 상태에서 두 번째 원소 GE가 적용되면 `GameplayEffectExecutionCalculation`이 두 태그의 조합을 감지해 반응 종류를 결정하고 반응 전용 GE를 실행한다.

### 원소 게이지 & Aura 소모 규칙

각 원소 Aura GE는 게이지를 `SetByCaller`의 값으로 보유한다. 반응 발생 시 두 게이지를 비교해 처리한다.

| 케이스 | 처리 |
|--------|------|
| 공격 원소 게이지 ≥ Aura 게이지 | Aura GE 완전 제거. 잔여 공격 게이지는 소멸 |
| 공격 원소 게이지 < Aura 게이지 | Aura GE의 게이지를 (Aura - 공격 * 반응계수)로 갱신 |

---

## 원소 반응 목록

| 반응 | 조합 | 효과 | GAS 구현 |
|------|------|------|----------|
| **증발 (Vaporize) / 융해 (Melt)** | 불 + 물 / 얼음 + 불 | 1.5~2× 피해 배율 | ExecutionCalc 내 적용 순서 플래그로 배율 분기 |
| **빙결 (Frozen)** | 물 + 얼음 | 이동 불가 | |
| **과부하 (Overloaded)** | 불 + 번개 | 범위 폭발 + 넉백 | ExecCalc에서 Gameplay Event 생성 → 반응 GA에서 Radial Damage GE + LaunchCharacter 실행 |
| **감전 (Electro-Charged)** | 번개 + 물 | 지속 피해, 수중 광역 확산 | Periodic GE + Overlap 기반 확산 |
| **확산 (Swirl)** | 바람 + (불·물·얼음·번개 중 하나) | 흡수 원소 광역 확산 | ApplyGameplayEffectToTarget 다중 호출 |
| **결정화 (Crystallize)** | 바위 + (불·물·얼음·번개 중 하나) | 방어막 생성 | ExecCalc에서 Gameplay Event 생성 → 반응 GA에서 Crystal Actor 스폰. 픽업 시 ShieldAbility 부여 |