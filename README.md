# CAN Gateway & UDS Diagnostic System

STM32 기반 다중 ECU CAN 네트워크 + UDS 진단 + VW Passat 실차 계기판 구동 프로젝트.

---

## 🏗 시스템 아키텍처

```
[보드A — 엔진 ECU + 페달]        CAN1 (Powertrain, 500kbps)
    │  가변저항 → RPM/Speed/Coolant 송신                │
    └──────────────────────────────────────────┐
                                                │
                              [보드B — Central Gateway]
                              CAN1 RX → 라우팅 → CAN2 TX
                              + 이상 감지 (RPM > 5500)
                              + VW 계기판 메시지 송신
                              + 트래픽 로거 (UART)
                                                │
    ┌──────────────────────────────────────────┘
    │              CAN2 (Diagnostic, 500kbps)
    ├──→ [보드C — UDS 진단 서버]  SID 0x22, UART CLI
    └──→ [VW Passat B6 계기판]    RPM/Speed/Temp 바늘 + 경고등
```

---

## 👥 팀 구성 및 담당 모듈

| 역할 | 담당자 | 담당 파일 | 브랜치 |
|---|---|---|---|
| ① CAN 드라이버 (공통) | 성재, 민진 | `common/can_bsp/` | `feature/can-bsp` |
| ② 엔진 ECU 시뮬 + 페달 | 윤지 | `board_a_engine/Core/Src/engine_sim.c` | `feature/engine-sim` |
| ③ 게이트웨이 + 이상 감지 | 지윤 | `board_b_gateway/Core/Src/gateway.c` | `feature/gateway` |
| ④ UDS 진단 서버 | 은빈 | `board_c_uds/Core/Src/uds_server.c` | `feature/uds-server` |
| ⑤ 계기판 인터페이스 | 미정 | `board_b_gateway/Core/Src/cluster_can.c` | `feature/cluster-can` |
| ⑥ RTOS + 통합 (팀장) | 한결 | 전체 조율 | `main` |

---

## 📁 폴더 구조

```
can-gateway-uds/
├── docs/                 # 아키텍처·CAN DB·UDS 매핑 문서
├── common/               # 전 보드 공유 코드 (팀장 관리)
│   ├── can_bsp/          # ① CAN 드라이버
│   ├── can_db/           # CAN ID·신호 정의 (계약서)
│   └── common_types/
├── board_a_engine/       # 보드A 프로젝트 (CubeIDE)
├── board_b_gateway/      # 보드B 프로젝트 (F446RE 필수)
├── board_c_uds/          # 보드C 프로젝트
├── tools/                # 테스트·모니터링 스크립트
└── tests/                # 단위 테스트
```

---

## 🚀 Quick Start

### 1. 클론
```bash
git clone https://github.com/<OWNER>/can-gateway-uds.git
cd can-gateway-uds
```

### 2. 자기 브랜치로 이동
```bash
git checkout feature/<your-module>
```

### 3. STM32CubeIDE에서 프로젝트 열기
- File → Import → Existing Projects into Workspace
- 자기 담당 보드 폴더 선택 (예: `board_a_engine/`)
- **common/ 폴더는 각 보드 프로젝트에서 Include Path로 추가됨** (자동 설정)

### 4. 빌드 & 플래시
- 보드 연결 → Build → Run

---

## 🌿 브랜치 전략

```
main                       ← 검증된 통합 (팀장만 머지)
├── feature/can-bsp        ← ① 성재/민진
├── feature/engine-sim     ← ② 윤지
├── feature/gateway        ← ③ 지윤
├── feature/uds-server     ← ④ 은빈
├── feature/cluster-can    ← ⑤ 미정
└── feature/integration    ← ⑥ 한결 (통합 테스트)
```

**규칙**
1. `main` 직접 push 금지 → PR 필수
2. `common/` 수정은 팀장 승인 필수
3. 커밋 메시지 컨벤션: `[모듈] 내용` (예: `[engine-sim] ADC 페달 입력 추가`)
4. PR 올리기 전 자기 브랜치에서 빌드 성공 확인

---

## 📋 CAN DB (요약)

전체 정의는 [docs/can_db.md](docs/can_db.md) 및 [common/can_db/can_db.h](common/can_db/can_db.h) 참조.

| 신호 | ID | 버스 | 주기 | 인코딩 |
|---|---|---|---|---|
| RPM | 0x280 | CAN1/CAN2 | 50ms | rpm × 4 |
| Speed | 0x1A0 | CAN1/CAN2 | 100ms | value × 0.01 km/h |
| Coolant Temp | 0x288 | CAN1/CAN2 | 1000ms | value - 40 °C |
| Warning | 0x480 | CAN2 | 이벤트 | byte[0] bitfield |
| UDS Req | 0x7DF | CAN2 | - | ISO 14229 |
| UDS Resp | 0x7E8 | CAN2 | - | ISO 14229 |

---

## 🛠 개발환경

- STM32CubeIDE 1.13+
- STM32CubeMX
- FreeRTOS (CMSIS-RTOS v2)
- HAL Driver
- USB-CAN 어댑터: CANable v2.0 (slcan)

---

## 📚 문서

- [시스템 아키텍처](docs/architecture.md)
- [CAN DB 정의](docs/can_db.md)
- [UDS DID 매핑](docs/uds_did_map.md)
- [인터페이스 규격](docs/interface_spec.md)

---

## 📅 일정

- **1주차**: 모듈 독립 개발 + 단위 테스트
- **2주차 전반**: 통합
- **2주차 후반**: 엔드투엔드 테스트 + 문서화 + 데모 영상

---

## 📝 License

Educational use only.
