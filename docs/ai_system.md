# AI 시스템

[← README로 돌아가기](../README.md)

---

## StateTree

- **보스 페이즈 전환**: HP 100% → 70% → 30% 임계값마다 Transition 발생. 각 Phase 진입 시 저항·속도·패턴 GE 자동 적용. `Boss.Phase.Two` 태그 부여로 전용 GA 잠금 해제.
- **수중 전투 전략**:  `Environment.Water` 태그 감지 시 `Combat_Underwater` 상태로 전이. 번개 계열 GA 우선 선택.
- **NPC 일상 루틴**: 시간대별 Morning/Work/Evening/Sleep 상태. 어촌 NPC는 낮에 수역 SmartObject(어업 GA) 예약, 밤에 귀환.

---

## BehaviorTree (AI 전술 행동)

- **전투 BT 구조**
- Selector: 
  1. 원거리 공격 가능 → 원거리 GA
  2.  근접 접근 → NavMesh 이동 → 근접 GA
  3.  대기
- **그룹 전술 BT**: 무리 몬스터는 Blackboard 공유. 1마리 탱킹 중 나머지 EQS로 측면 포지션
- **AI 인식 시스템**: AIPerception 시각·청각·피해 감지

---

## 적 유형별 AI 아키텍처 매핑

| 적 유형 | 주요 AI | SmartObject 활용 | GAS 연동 |
|---------|---------|-----------------|----------|
| 일반 몬스터 | BehaviorTree + EQS | 폭발물·엄폐물 전술 | BT Task에서 TryActivateAbility 호출 |
| 엘리트 | StateTree + BT| 아레나 기믹 스위치 | Phase Tag로 추가 GA 잠금 해제 |
| 보스 | StateTree (메인) + BT (서브) | Phase별 전용 아레나 SmartObject | Phase GE로 저항·속도·패턴 전환 |
| 마을 NPC | StateTree + SmartObject | 일상 루틴 슬롯 예약·사용 | 퀘스트 WorldStateTag로 Ability 변경 |
| 도시 군중 | MassEntity + SmartObject | 이동 경로·대기 지점 | 성능을 위해 GAS 미사용 |
