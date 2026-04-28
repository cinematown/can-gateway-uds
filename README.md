# CAN Gateway & UDS Diagnostic System

STM32F429ZI 기반 CAN 게이트웨이, UDS 진단 CLI, VW 계기판 신호 실험 프로젝트.

## Structure

```text
can-gateway-uds/
├── common/
│   ├── can_bsp.c/h
│   ├── cli.c/h
│   ├── protocol_ids.h
│   ├── signal_db.h
│   └── uart.c/h
├── docs/
│   ├── VW_Passat_B6.dbc
│   ├── architecture.md
│   ├── can_db.md
│   ├── pinout_cluster.md
│   ├── signal_db.txt
│   └── uds_did_map.md
├── firmware/
│   ├── board_a_engine/
│   │   ├── CMakeLists.txt
│   │   ├── Core/ Drivers/ Middlewares/
│   │   ├── board_a_engine.ioc
│   │   └── src/
│   ├── board_b_gateway/
│   │   ├── CMakeLists.txt
│   │   ├── Core/ Drivers/ Middlewares/
│   │   ├── board_b_gateway.ioc
│   │   └── src/
│   └── board_c_uds/
│       ├── CMakeLists.txt
│       ├── Core/ Drivers/ Middlewares/
│       ├── board_c_uds.ioc
│       └── src/
├── tools/
│   ├── can_monitor/
│   └── dbc_generator/
└── CMakeLists.txt
```

모든 펌웨어 보드는 STM32F429ZI 기준입니다. 공통 HAL 의존 앱 코드는 `common/`에 두고, 보드별 동작만 각 `firmware/board_*/src`에 둡니다.

## Boards

| Board | Role | Main source |
|---|---|---|
| A | Engine simulator, ADC throttle, CAN1 TX | `firmware/board_a_engine/src/engine_sim.c` |
| B | CAN1/CAN2 gateway, warning routing | `firmware/board_b_gateway/src/gateway_main.c` |
| C | UDS diagnostic CLI imported from the refactored UDS project | `firmware/board_c_uds/src/ap_main.c` |

## Build

보드별 독립 빌드:

```bash
cmake --preset Debug --fresh -S firmware/board_a_engine
cmake --build firmware/board_a_engine/build/Debug

cmake --preset Debug --fresh -S firmware/board_b_gateway
cmake --build firmware/board_b_gateway/build/Debug

cmake --preset Debug --fresh -S firmware/board_c_uds
cmake --build firmware/board_c_uds/build/Debug
```

루트에서 전체 보드 빌드:

```bash
cmake --preset Debug
cmake --build --preset Debug
```

필요 도구: `cmake`, `make`, `arm-none-eabi-gcc`.

## CAN IDs

| Signal | CAN ID | Source |
|---|---:|---|
| Engine RPM | `0x280` | `VW_Passat_B6.dbc`, `Motor_1.Motordrehzahl` |
| Vehicle speed | `0x1A0` | `VW_Passat_B6.dbc`, `Bremse_1.BR1_Rad_kmh` |
| Coolant temp | `0x288` | `VW_Passat_B6.dbc`, `Motor_2.MO2_Kuehlm_T` |
| Warning | `0x480` | Project gateway warning |
| UDS request | `0x714` | Board C -> gateway |
| UDS response | `0x77E` | Gateway/target -> Board C |
