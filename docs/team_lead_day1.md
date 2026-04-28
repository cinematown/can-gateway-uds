# 팀장용 — Day 1 셋업 체크리스트

이 문서는 팀장(⑥ 한결)이 **프로젝트 시작일에 반드시 완료해야 할 작업**입니다.
여기까지 끝나야 나머지 5명이 병렬 작업을 시작할 수 있습니다.

---

## ✅ Day 1 오전 (2~3시간)

### 1. GitHub 리포 생성 & 초기 push
```bash
# 깃허브에서 Private 리포 생성 (이름: can-gateway-uds)

cd can-gateway-uds        # 이 스캐폴드 압축 풀린 폴더
git init
git add .
git commit -m "[init] Initial project scaffold

- common/signal_db.h: DBC 기반 CAN ID/신호 정의
- common/can_bsp: CAN 드라이버 인터페이스 + stub 구현
- board_a/b/c: 보드별 프로젝트 골격 + 모듈 header/stub
- docs: architecture, CAN DB, UDS DID map, interface spec
- CONTRIBUTING, PR/Issue 템플릿"
git branch -M main
git remote add origin https://github.com/<OWNER>/can-gateway-uds.git
git push -u origin main
```

### 2. 팀원 브랜치 5개 생성
```bash
for BR in feature/can-bsp feature/engine-sim feature/gateway \
          feature/uds-server feature/cluster-can feature/integration; do
  git checkout -b $BR main
  git push -u origin $BR
done
git checkout main
```

### 3. GitHub 설정 (웹)
- **Settings → Branches → Add branch protection rule**
  - Branch name pattern: `main`
  - ☑ Require a pull request before merging
  - ☑ Require approvals (1명)
  - ☑ Do not allow bypassing the above settings
- **Settings → Collaborators** → 팀원 5명 초대 (Write 권한)
- **Issues** → 탭 활성화 확인

### 4. 팀원 초대 메시지 (템플릿)
```
@팀원들 GitHub 초대 전송했습니다.

1. 초대 수락 후: git clone https://github.com/<OWNER>/can-gateway-uds.git
2. 자기 브랜치 체크아웃: git checkout feature/<본인-모듈>
3. README.md → CONTRIBUTING.md → docs/interface_spec.md 순서로 읽기
4. 자기 담당 보드 폴더의 README도 확인
5. 막히는 거 있으면 바로 DM

Day 1 밤까지: CubeMX 프로젝트 생성 + 보드 플래시 Hello World 성공
Day 2 아침: 인터페이스 헤더 최종 확정본 공유합니다.
```

---

## ✅ Day 1 오후 (2~3시간)

### 5. 보드 3개 CubeMX 프로젝트 생성
각 `board_*_*/` 폴더에 들어가서 CubeMX 로 `.ioc` 생성.

**공통 설정**:
- RCC: HSE Crystal/Ceramic Resonator (Nucleo 기본)
- SYS: Serial Wire
- FreeRTOS (CMSIS v2) 활성화
- USART2: Asynchronous, 115200 8N1 (디버그 출력)

**보드A 추가**:
- CAN1 활성화, 500kbps (Prescaler=5, BS1=13, BS2=2 @ APB1=45MHz)
- ADC1 Channel 0, 1 (Continuous mode, DMA)

**보드B 추가**:
- CAN1 활성화 (Powertrain)
- CAN2 활성화 (Diagnostic) — STM32F429ZI 기준
- 둘 다 500kbps

**보드C 추가**:
- CAN1 활성화, 500kbps
- USART2 Interrupt 활성화 (CLI 입력용)

**CubeMX 생성 후 각 폴더에**:
- `Drivers/`, `Core/`, `Middlewares/` 가 자동 생성됨
- 기존 `Core/Inc/*.h`, `Core/Src/*.c` stub 들은 유지 (CubeMX 가 덮어쓰지 않는 `USER CODE` 영역에 있음)

### 6. Include Path 설정
각 보드 프로젝트에서:
- 프로젝트 우클릭 → Properties → C/C++ Build → Settings
- Tool Settings → MCU GCC Compiler → Include paths
- 추가: `"../../common"`

### 7. 소스 링크 추가
각 보드 프로젝트에서:
- 프로젝트 우클릭 → New → Folder → Advanced → Link to alternate location
- `../../common/can_bsp` 링크

### 8. 빌드 확인
- 보드 3개 모두 빌드 성공 (warning 있어도 OK)
- Nucleo에 플래시 → ST-Link VCP 로 "Hello CAN Gateway" 출력 확인

### 9. Day 1 마무리 커밋
```bash
git add .
git commit -m "[build] Board A/B/C CubeMX projects generated

- CAN1/CAN2 @ 500kbps
- FreeRTOS CMSIS v2
- USART2 debug log confirmed on all boards"
git push origin main  # 팀장만 가능
```

---

## ✅ Day 2 — 인터페이스 확정 및 배포

### 10. 헤더 파일 최종 점검
팀원들 피드백 받고 필요 시 수정:
- `common/signal_db.h`, `common/protocol_ids.h` — CAN ID, DID, 신호 매크로
- `common/can_bsp/can_bsp.h` — API 시그니처
- 각 모듈 `*.h` — 외부 공개 API

### 11. Day 2 공지
```
@팀원들 인터페이스 확정됐습니다.

✅ 확정된 파일 (수정 시 팀장 승인 필수):
  - common/signal_db.h
  - common/can_bsp/can_bsp.h
  - 각 모듈 header (Inc/*.h)

🔨 구현 시작:
  - 각자 자기 브랜치에서 .c 파일 작성
  - stub (printf 출력) 으로 자체 테스트 가능
  - Day 4 중간 체크 (특히 ① CAN 드라이버 — 크리티컬 패스)

💬 막히면:
  - GitHub Issue 생성 ([interface] 태그면 긴급)
  - 카톡/Slack DM
```

---

## ✅ Day 4 중간 체크 (⚠️ 크리티컬)

### 12. ① CAN 드라이버 상태 확인
- 실 CAN 양방향 통신 성공했는가?
- 2대 Nucleo + TJA1050 2개 + 120Ω 종단저항으로 loopback 성공?
- 안 됐으면 **이날 팀장이 들어가서 같이 디버깅**. 이게 밀리면 전체 일정 밀림.

### 13. 나머지 모듈 진행률 확인
- ② 엔진 시뮬: ADC 읽기 성공?
- ③ 게이트웨이: 라우팅 테이블 구조 완성?
- ④ UDS: SID 0x22 핸들러 로직 완성?
- ⑤ 계기판: 12V 전원 인가, keep-alive 메시지 송신 확인?

각자 UART 로그 스크린샷을 Slack/카톡에 올리게 해서 증빙.

---

## 🚨 자주 터지는 문제 (예방)

| 문제 | 예방 |
|---|---|
| `.ioc` 머지 충돌 | `.ioc` 수정은 팀장만. 수정하면 즉시 공지 + 전원 pull |
| Include path 누락 | Day 1 마지막에 팀장이 한번에 설정 + 커밋 |
| CAN bit timing 불일치 | `docs/can_db.md` 의 Prescaler/BS1/BS2 값 공유 |
| Queue 용량 부족 | depth 16 고정 (트래픽 많으면 32로 상향) |
| HAL 버전 불일치 | CubeMX 버전 통일 (팀장 버전 공지) |
| 팀원이 `common/` 임의 수정 | 브랜치 보호 + PR 리뷰로 차단 |
