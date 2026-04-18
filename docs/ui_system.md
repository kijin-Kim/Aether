## UI 시스템

### UI 최적화

| 기술 요소 | 설계 내용 | 구현 방식 |
|-----------|-----------|----------|
| **메모리 & 로딩 최적화** | 하드레퍼런스 제거. 뷰포트 진입 직전 비동기 로드 | `TSoftObjectPtr<UTexture2D>` + `RequestAsyncLoad()` 활용 |
| **Tick 배제 UI** | 상태 변화 시에만 위젯 갱신 | `GetGameplayAttributeValueChangeDelegate` 바인딩을 통해 Event-Driven 구조 확립 |
| **위젯 풀링 (Pooling)** | 스크롤 시 위젯 재생성 방지 | 초기 슬롯 50개 생성 후 `SetVisibility` 및 데이터 교체 기반으로 재사용 |
| **Invalidation Box** | 정적 영역 드로우 콜 방지 | 갱신 빈도 낮은 위젯 `UInvalidationBox` 래핑. 명시적 `Invalidate()` 호출 |
