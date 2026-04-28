## 변경 요약
<!-- 이 PR이 무엇을 하는지 2~3줄로 설명 -->


## 변경 모듈
- [ ] ① CAN 드라이버 (`common/can_bsp.*`)
- [ ] ② 엔진 시뮬 (`firmware/board_a_engine/src`)
- [ ] ③ 게이트웨이 (`firmware/board_b_gateway/src`)
- [ ] ④ UDS 서버 (`firmware/board_c_uds/src`)
- [ ] ⑤ 계기판 (`firmware/board_b_gateway/src/cluster_*`, 추가 예정)
- [ ] 공통 인터페이스 (`common/signal_db.h`, `common/protocol_ids.h`, `common/can_bsp.h`) ⚠️ 팀장 승인 필수
- [ ] 문서 (`docs/`, `README.md`)
- [ ] 빌드 / CubeMX 설정

## 테스트
<!-- 어떻게 동작을 확인했는지 -->
- [ ] 담당 보드 단위 빌드 성공
- [ ] 루트 통합 빌드 성공
- [ ] Loopback 테스트 통과 (해당 시)
- [ ] 실 CAN 통신 테스트 통과 (해당 시)
- [ ] UART 로그로 기대 동작 확인

### 테스트 로그 / 스크린샷
<!-- 가능하면 붙여주세요 -->
```
[여기에 UART 로그 붙여넣기]
```

## 인터페이스 변경 여부
- [ ] **인터페이스 변경 없음** (구현만 수정)
- [ ] 인터페이스 변경 있음 → 영향받는 담당자 리뷰어에 추가 필수

## 체크리스트
- [ ] 하드코딩된 CAN ID 없음 (`signal_db.h` / `protocol_ids.h` 매크로 사용)
- [ ] `TODO:` 주석에 미완성 항목 명시
- [ ] 커밋 메시지 `[모듈] 요약` 형식
- [ ] 관련 문서 업데이트 (필요 시)

## 남은 작업 / 다음 PR 예정
<!-- 이 PR 이후 해야 할 일이 있으면 기록 -->
