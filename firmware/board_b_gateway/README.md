# Board B - Central Gateway

Board B는 CAN1의 Engine/Body 보드 메시지를 수신하고, CAN2의 계기판/진단 버스로 필요한 프레임을 송신하는 Central Gateway입니다.

## 개요

```text
CAN1 500kbps
  Board A Engine  -> 0x100 EngineData
  Board D Body    -> 0x390 Golf6 mGate_Komf_1
        |
        v
Board B Gateway
  CAN1 RX queue
  GatewayTask
  Engine/Body bridge
        |
        v
CAN2 500kbps
  0x280 Motor_1 RPM
  0x1A0 Bremse_1 Speed
  0x390 Body / mGate_Komf_1 forwarding
  UART CLI CAN monitor
```

## 주요 기능

| 기능 | 설명 |
|---|---|
| CAN1/CAN2 BSP | CAN1, CAN2 open filter 설정, RX interrupt, RTOS queue 기반 수신 |
| Engine bridge | Board A `0x100`을 수신해 계기판용 `0x280`, `0x1A0`으로 변환 송신 |
| Body bridge | Board D Golf6 `0x390`을 수신해 CAN2로 즉시/주기 포워딩 |
| Gateway task | CAN RX queue를 읽고 bridge 모듈에 프레임 전달 |
| Logger task | RX/TX 카운터와 warning 상태를 UART로 출력 |
| CAN CLI monitor | `canlog` 명령으로 특정 CAN ID 로그 필터링 |

## 파일 구조

| 파일 | 역할 |
|---|---|
| `app/gateway_tasks.c` | Gateway 전체 task 허브. CAN 초기화, RX queue 처리, bridge 호출, 상태 로그 |
| `app/gateway_engine_bridge.c/h` | Board A `0x100` 엔진 데이터를 계기판용 CAN2 프레임으로 변환 |
| `app/gateway_body_bridge.c/h` | Board D `0x390` Body 프레임을 CAN2로 포워딩 |
| `app/can_bsp.c/h` | CAN1/CAN2 초기화, 송신, RX interrupt queue 처리 |
| `app/can_cli_monitor.c/h` | UART CLI 기반 CAN frame 로그 필터 |
| `app/cli.c/h` | UART CLI 명령 프레임워크 |
| `app/uart.c/h` | UART3 RX queue, TX mutex, printf 출력 |
| `app/protocol_ids.h` | Gateway 앱에서 사용하는 CAN ID, payload index 정의 |
| `app/signal_db.h` | 계기판용 signal encoding helper |

## CAN 입력/출력

### CAN1 입력

| 송신 보드 | CAN ID | DLC | 의미 |
|---|---:|---:|---|
| Board A Engine | `0x100` | 8 | RPM, Speed, Coolant, IGN, alive counter |
| Board D Body | `0x390` | 8 | Golf6 `mGate_Komf_1` Body 상태 |

### CAN2 출력

| CAN ID | 주기/조건 | 설명 |
|---:|---|---|
| `0x280` | 50ms | 계기판 RPM, `Motor_1` |
| `0x1A0` | 코드 기준 500ms | 계기판 Speed, `Bremse_1` |
| `0x390` | 수신 즉시 + 100ms keep-alive | Board D Body frame 포워딩 |
| `0x100` | Board A 수신 시 | EngineData 원본 포워딩 및 warning 판단 경로 |

## EngineData `0x100`

Board A가 CAN1으로 보내는 프로젝트 내부 엔진 프레임입니다.

| Byte | 의미 |
|---|---|
| `0-1` | RPM, `uint16_t`, little-endian |
| `2-3` | Speed km/h, `uint16_t`, little-endian |
| `4` | Coolant temperature |
| `5` | Status: bit0 IGN ON, bit1-7 alive counter |
| `6-7` | Reserved |

Gateway는 이 값을 `ClusterInputState_t`에 저장하고, CAN2 계기판 프레임으로 변환합니다. IGN OFF이거나 마지막 수신 후 1000ms가 지나면 RPM/Speed를 0으로 송신합니다.

## Body `0x390`

Board D는 Golf6 DBC 기반 `mGate_Komf_1` 프레임을 `0x390`으로 송신합니다. Gateway의 `CAN_ID_BODY_STATUS`도 `0x390`으로 맞춰져 있습니다.

Body bridge 동작:

1. CAN1에서 `id == 0x390`, `dlc == 8`인 프레임만 처리
2. payload 8바이트를 최신 Body 상태로 저장
3. 수신 즉시 CAN2 `0x390`으로 송신
4. 마지막 수신 후 500ms 이내이면 100ms마다 CAN2로 재송신

## UART CLI

USART3, 115200 8N1 기준입니다.

부팅 메시지:

```text
[GW] UART CLI ready. type 'help', 'log off', or 'canlog stat'
CLI >
```

### 상태 로그

기본적으로 1초마다 Gateway counter가 출력됩니다.

```text
[GW] RX1=1811 TX1=0 RX2=2 TX2=4572 busy=0 err=0 warn=0
```

끄기:

```text
log off
```

켜기:

```text
log on
```

### CAN 로그

특정 ID만 보기:

```text
canlog id 100
canlog on
```

Body D 메시지 확인:

```text
log off
canlog id 390
canlog on
```

기대 로그:

```text
[RX1] id=0x390 dlc=8 data=...
[TX2] id=0x390 dlc=8 data=... st=0
```

Engine A 메시지 확인:

```text
canlog off
canlog id 100
canlog on
```

계기판 출력 확인:

```text
canlog off
canlog id 280
canlog on
```

```text
canlog off
canlog id 1A0
canlog on
```

전체 로그:

```text
canlog all
canlog on
```

## 통합 확인 순서

1. Board B UART에서 Gateway init 확인
2. `log off`
3. Board A 수신 확인
   ```text
   canlog id 100
   canlog on
   ```
   기대: `[RX1] id=0x100 ...`
4. Board D 수신 확인
   ```text
   canlog off
   canlog id 390
   canlog on
   ```
   기대: `[RX1] id=0x390 ...`
5. 계기판 CAN2 송신 확인
   ```text
   canlog off
   canlog id 280
   canlog on
   ```
   기대: `[TX2] id=0x280 ...`

## 트러블슈팅

| 증상 | 확인 |
|---|---|
| `RX1`은 증가하지만 `0x390`이 안 보임 | `RX1` 증가가 Board A `0x100`인지 먼저 확인. Board D UART에서 `CAN1 TX OK id=0x390` 확인 |
| `[RX1] id=0x390`은 보이는데 `[TX2] id=0x390`이 안 보임 | Board B가 최신 gateway body bridge 코드로 플래시됐는지 확인 |
| `0x100`도 안 보임 | CAN1 배선, CANH/CANL, GND, 종단저항, bitrate 확인 |
| D가 `0x390`을 송신하지 않음 | Board D IGN 조건 확인. D UART에서 `IGN ON` 로그 확인 |
| UART 로그가 너무 많음 | `log off`, `canlog id <id>`로 필터 후 `canlog on` 사용 |

## 빌드/플래시

보드별로 STM32CubeIDE 또는 CMake preset을 사용합니다.

```sh
cmake --preset Debug
cmake --build --preset Debug --parallel
```

생성된 ELF를 Board B에 플래시한 뒤 UART CLI에서 통신을 확인합니다.
