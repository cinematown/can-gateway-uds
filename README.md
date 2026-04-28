# CAN Gateway & UDS Diagnostic System

STM32F429ZI 기반 다중 ECU CAN 네트워크 + UDS 진단 + VW Passat 실차 계기판 구동 프로젝트.

---

## 시스템 아키텍처

```text
CAN1 (Powertrain/Body, 500kbps)
    [보드A - Engine ECU + 페달]  -> RPM/Speed/Coolant/IGN 송신
    [보드D - Body / BCM]         -> Door/Turn/High/Fog status 송신
                    |
                    v
          [보드B - Central Gateway]
          CAN1 RX -> 라우팅 -> CAN2 TX
          + 이상 감지 (RPM >= 5000)
          + VW 계기판 메시지 송신
          + 트래픽 로거 (UART)
                    |
                    v
CAN2 (Diagnostic/Cluster, 500kbps)
    +--> [보드C - UDS 진단 CLI]  SID 0x22, UART CLI
    +--> [VW Passat B6 계기판]   RPM/Speed/Temp/Body/Warning
```

---

## 팀 구성 및 담당 모듈

| 역할 | 담당자 | 담당 파일 | 브랜치 |
|---|---|---|---|
| ① CAN 드라이버 (공통) | 성재, 민진 | `common/can_bsp.c`, `common/can_bsp.h` | `feature/can-bsp` |
| ② 엔진 ECU 시뮬 + 페달 | 윤지 | `firmware/board_a_engine/src/engine_sim.c` | `feature/engine-sim` |
| ③ 게이트웨이 + 이상 감지 | 지윤 | `firmware/board_b_gateway/src/gateway_main.c` | `feature/gateway` |
| ④ UDS 진단 CLI | 은빈 | `firmware/board_c_uds/src/uds_service.c`, `firmware/board_c_uds/src/uds_server.c` | `feature/uds-server` |
| ⑤ Body / BCM | 미정 | `firmware/board_d_body/src/bcm_main.c` | `feature/body-bcm` |
| ⑥ 계기판 인터페이스 | 미정 | `firmware/board_b_gateway/src/cluster_can.c` 예정 | `feature/cluster-can` |
| ⑦ RTOS + 통합 (팀장) | 한결 | 전체 조율, `CMakeLists.txt`, GitHub Actions | `main` |

---

## 폴더 구조

```text
can-gateway-uds/
├── docs/                         # 아키텍처, CAN DB, UDS 매핑, DBC 문서
│   ├── VW_Passat_B6.dbc
│   ├── architecture.md
│   ├── can_db.md
│   ├── interface_spec.md
│   ├── pinout_cluster.md
│   ├── signal_db.txt
│   └── uds_did_map.md
├── common/                       # 전 보드 공유 코드
│   ├── can_bsp.c/h               # CAN 초기화, 송수신, RX 큐
│   ├── cli.c/h                   # UART CLI 공통 프레임워크
│   ├── protocol_ids.h            # 프로젝트 CAN ID, UDS DID
│   ├── signal_db.h               # DBC 기반 신호 정의/인코더
│   └── uart.c/h                  # UART3 RX 큐, TX mutex
├── firmware/
│   ├── board_a_engine/           # 보드A F429ZI 프로젝트
│   │   ├── CMakeLists.txt
│   │   ├── Core/ Drivers/ Middlewares/
│   │   ├── board_a_engine.ioc
│   │   └── src/
│   ├── board_b_gateway/          # 보드B F429ZI 프로젝트
│   │   ├── CMakeLists.txt
│   │   ├── Core/ Drivers/ Middlewares/
│   │   ├── board_b_gateway.ioc
│   │   └── src/
│   ├── board_c_uds/              # 보드C F429ZI UDS 프로젝트
│   │   ├── CMakeLists.txt
│   │   ├── Core/ Drivers/ Middlewares/
│   │   ├── board_c_uds.ioc
│   │   └── src/
│   └── board_d_body/             # 보드D F429ZI Body/BCM 프로젝트
│       ├── CMakeLists.txt
│       ├── Core/ Drivers/ Middlewares/
│       ├── board_d_body.ioc
│       └── src/
├── tools/
│   ├── can_monitor/
│   └── dbc_generator/
├── tests/
└── CMakeLists.txt                # 루트 통합 빌드 관리
```

---

## Quick Start

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

- File -> Import -> Existing Projects into Workspace
- 자기 담당 보드 폴더 선택
  - `firmware/board_a_engine`
  - `firmware/board_b_gateway`
  - `firmware/board_c_uds`
  - `firmware/board_d_body`
- `common/` 폴더는 각 보드 `CMakeLists.txt`에서 include/source로 연결됨

### 4. 로컬 빌드

보드별 단위 빌드:

```bash
cmake --preset Debug --fresh -S firmware/board_a_engine
cmake --build firmware/board_a_engine/build/Debug --parallel

cmake --preset Debug --fresh -S firmware/board_b_gateway
cmake --build firmware/board_b_gateway/build/Debug --parallel

cmake --preset Debug --fresh -S firmware/board_c_uds
cmake --build firmware/board_c_uds/build/Debug --parallel

cmake --preset Debug --fresh -S firmware/board_d_body
cmake --build firmware/board_d_body/build/Debug --parallel
```

루트 통합 빌드:

```bash
cmake --preset Debug --fresh
cmake --build --preset Debug --parallel
```

### 5. 플래시

- 보드 연결
- STM32CubeIDE 또는 STM32CubeProgrammer로 각 보드의 `.elf` 다운로드
- 빌드 결과 위치 예:
  - `firmware/board_a_engine/build/Debug/board_a_engine.elf`
  - `firmware/board_b_gateway/build/Debug/board_b_gateway.elf`
  - `firmware/board_c_uds/build/Debug/board_c_uds.elf`
  - `firmware/board_d_body/build/Debug/board_d_body.elf`

---

## 브랜치 전략

```text
main                       <- 검증된 통합
├── feature/can-bsp        <- ① 성재/민진
├── feature/engine-sim     <- ② 윤지
├── feature/gateway        <- ③ 지윤
├── feature/uds-server     <- ④ 은빈
├── feature/body-bcm       <- ⑤ 미정
├── feature/cluster-can    <- ⑥ 미정
└── feature/integration    <- ⑦ 한결
```

규칙:

1. `main` 직접 push 금지 -> PR 필수.
2. `common/` 수정은 팀장 승인 필수.
3. 커밋 메시지 컨벤션: `[모듈] 내용` 예: `[engine-sim] ADC 페달 입력 추가`.
4. PR 올리기 전 담당 보드 단위 빌드 성공 확인.
5. `common/` 또는 빌드 설정 수정 시 루트 통합 빌드 성공 확인.

---

## CAN DB 요약

전체 정의는 [docs/can_db.md](docs/can_db.md), [docs/signal_db.txt](docs/signal_db.txt), [common/signal_db.h](common/signal_db.h)를 참조합니다.
원본 DBC는 [docs/VW_Passat_B6.dbc](docs/VW_Passat_B6.dbc)입니다.

| 신호 | ID | 버스 | 주기 | 인코딩 |
|---|---:|---|---|---|
| RPM | `0x280` | CAN1/CAN2 | 50ms | `Motor_1.Motordrehzahl`, raw = rpm x 4 |
| Speed | `0x1A0` | CAN1/CAN2 | 100ms | `Bremse_1.BR1_Rad_kmh`, raw = km/h x 100 |
| Coolant Temp | `0x288` | CAN1/CAN2 | 1000ms | `Motor_2.MO2_Kuehlm_T`, raw = (degC + 48) / 0.75 |
| Warning | `0x480` | CAN2 | 이벤트 | byte[0] bitfield |
| Body Status | `0x470` | CAN1/CAN2 | 100ms | byte[0] bitfield: turn/door/high/fog |
| IGN/Keepalive | `0x300` | CAN1 | 100ms | byte[0] bit0 = IGN ON |
| UDS Req | `0x714` | CAN2/CAN1 routed | 이벤트 | ISO 14229 SID 0x22 Single Frame |
| UDS Resp | `0x77E` | CAN1/CAN2 routed | 이벤트 | ISO 14229 positive/negative response |

---

## 개발환경

- STM32F429ZI 보드 4대
- STM32CubeIDE / STM32CubeMX
- STM32CubeProgrammer
- CMake 3.22+
- Arm GNU Toolchain (`arm-none-eabi-gcc`, `arm-none-eabi-g++`)
- FreeRTOS (CMSIS-RTOS v2)
- STM32 HAL Driver
- USB-CAN 어댑터: CANable v2.0 또는 호환 slcan 장치

---

## GitHub Actions

PR과 `main` push 시 [.github/workflows/firmware-ci.yml](.github/workflows/firmware-ci.yml)이 실행됩니다.

| Job | 역할 |
|---|---|
| `layout` | 필수 폴더/파일 구조 검증 |
| `unit-build` | 보드 A/B/C/D 단위 빌드 |
| `integration-build` | 루트 통합 빌드 |

---

## 문서

- [시스템 아키텍처](docs/architecture.md)
- [CAN DB 정의](docs/can_db.md)
- [UDS DID 매핑](docs/uds_did_map.md)
- [인터페이스 규격](docs/interface_spec.md)
- [계기판 핀아웃](docs/pinout_cluster.md)
- [기여/빌드 가이드](CONTRIBUTING.md)

---

## 일정

- 1주차: 모듈 독립 개발 + 단위 테스트
- 2주차 전반: CAN1/CAN2 쌍별 통합
- 2주차 후반: 엔드투엔드 테스트 + 문서화 + 데모 영상

---

## License

Educational use only.
