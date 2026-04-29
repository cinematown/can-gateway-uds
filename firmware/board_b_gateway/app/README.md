# Gateway Cluster Bridge 설명서

이 문서는 Board B Gateway의 계기판 송신 기능을 설명합니다.

기존 `gateway_tasks.c`는 전체 CAN gateway task 역할을 유지하고, `gateway_cluster_bridge.c`는 계기판 변환만 담당합니다.

현재 구현 파일:

- `gateway_cluster_bridge.c`
- `gateway_cluster_bridge.h`

## 전체 역할

Board A는 자체 프로젝트 포맷으로 엔진 데이터를 CAN1에 송신합니다.

Board B Gateway는 이 메시지를 받아서 VW 계기판이 이해할 수 있는 CAN2 메시지로 변환합니다.

흐름은 아래와 같습니다.

```text
Board A
  CAN1 0x100, protocol_ids.h 포맷
        |
        v
Board B Gateway app/gateway_cluster_bridge.c
  rpm, speed, ignition 상태 해석
        |
        v
CAN2 계기판용 메시지 송신
  0x280 Motor_1  : RPM 바늘
  0x1A0 Bremse_1 : Speed 바늘
```

## 기존 gateway_tasks.c와의 관계

CubeMX가 생성한 `Core/Src/freertos.c`는 Gateway task로 `GatewayTask()`를 실행합니다.

```c
GatewayTask_Handle = osThreadNew(GatewayTask, NULL, &GatewayTask__attributes);
```

`GatewayTask()` 자체는 기존 `app/gateway_tasks.c`에 있습니다.

그래서 `gateway_cluster_bridge.c`는 task를 새로 만들거나 override하지 않고, 기존 task에서 호출되는 보조 모듈로 동작합니다.

연결 지점은 두 곳입니다.

| 위치 | 호출 함수 | 의미 |
|---|---|---|
| `GatewayTask()` | `GatewayClusterBridge_OnRx(&rxMsg)` | Board A `0x100` 수신값을 계기판용 최신 상태로 저장 |
| `ClusterTask()` | `GatewayClusterBridge_Task10ms()` | 저장된 최신 상태를 CAN2 `0x280/0x1A0`로 주기 송신 |

이 구조 덕분에 CAN 초기화, queue 처리, logger 등 기존 gateway 동작은 그대로 유지됩니다.

## Board A 입력 메시지

Board A에서 들어오는 메시지는 `app/protocol_ids.h` 기준입니다.

현재 입력 CAN ID:

```c
CAN_ID_ENGINE_DATA = 0x100
```

payload 의미:

| Byte | 의미 |
|---|---|
| `byte0-1` | RPM, `uint16_t`, little-endian |
| `byte2-3` | Speed, `uint16_t`, little-endian |
| `byte4` | Coolant temperature |
| `byte5` | Status |
| `byte6-7` | Reserved |

Status byte 의미:

| Bit | 의미 |
|---|---|
| `bit0` | `ign_on`, ignition ON 상태 |
| `bit1-7` | Board A alive counter |

## 주요 변수 의미

### `ClusterInputState_t`

Board A에서 마지막으로 받은 값을 저장하는 구조체입니다.

계기판은 한 번만 메시지를 받는 것이 아니라 일정 주기로 계속 메시지를 받아야 바늘을 유지합니다.

그래서 CAN1 수신 순간에 바로 CAN2로 한 번 보내고 끝내지 않고, 최신 값을 저장한 뒤 주기적으로 CAN2에 재송신합니다.

| 변수 | 의미 |
|---|---|
| `rpm` | Board A에서 받은 엔진 RPM |
| `speed_kmh` | Board A에서 받은 차량 속도, km/h |
| `coolant_c` | 냉각수 온도, 현재는 저장만 하고 계기판 송신에는 미사용 |
| `board_a_alive` | Board A가 보내는 alive counter |
| `ign_on` | 시동/IGN ON 상태 |
| `last_rx_tick` | 마지막으로 정상 Board A 메시지를 받은 RTOS tick |

### `ign_on`

Ignition ON 상태입니다.

`protocol_ids.h`의 `CAN_ENGINE_STATUS_IGN_MASK`로 status byte의 bit0을 읽어서 판단합니다.

`ign_on == false`이면 계기판에 rpm/speed를 0으로 내려 보냅니다.

### `board_a_alive`

Board A가 살아서 계속 메시지를 보내고 있는지 확인하기 위한 counter 값입니다.

현재 구현에서는 저장만 하고 직접 검증에는 쓰지 않습니다.

추후 Board A 메시지가 멈췄는지, counter가 증가하지 않는지 감시할 때 사용할 수 있습니다.

### `s_bremse_alive_counter`

계기판용 `0x1A0 Bremse_1` 프레임의 byte7 하위 4비트에 넣는 counter입니다.

사용자가 말한 것처럼 Speed 바늘은 특정 바이트가 계속 변하지 않으면 계기판이 메시지를 죽은 값으로 보고 바늘을 0으로 떨어뜨릴 수 있습니다.

그래서 `0, 1, 2, ... 15, 0`으로 반복 증가시킵니다.

## Bremse 뜻

`Bremse`는 독일어로 `브레이크`입니다.

VW DBC에서 `0x1A0` 메시지 이름이 `Bremse_1`로 되어 있습니다.

속도 신호 `BR1_Rad_kmh`가 이 브레이크/ABS 계열 메시지 안에 들어있기 때문에, 코드에서도 DBC 이름을 따라 `BREMSE_1`이라는 이름을 사용했습니다.

즉 여기서 `Bremse_1`은 브레이크를 직접 제어한다는 뜻이 아니라, VW 계기판이 속도값을 기대하는 CAN 메시지 이름입니다.

## CAN2 송신 메시지

### RPM 메시지

CAN ID:

```c
0x280
```

DBC 이름:

```text
Motor_1
```

역할:

```text
RPM 계기판 바늘
```

인코딩:

```text
raw = rpm * 4
start bit = 16
length = 16
little-endian
```

예를 들어 `rpm = 1350`이면:

```text
raw = 1350 * 4 = 5400 = 0x1518
```

byte2-3 근처에 이 raw 값이 들어갑니다.

사용자가 준 예시:

```text
0x280  8  00 00 15 15 00 00 00 00
```

### Speed 메시지

CAN ID:

```c
0x1A0
```

DBC 이름:

```text
Bremse_1
```

역할:

```text
Speed 계기판 바늘
```

인코딩:

```text
raw = speed_kmh * 100
start bit = 17
length = 15
little-endian
```

예를 들어 `speed_kmh = 100`이면:

```text
raw = 100 * 100 = 10000 = 0x2710
```

start bit이 17이라 raw 값이 1비트 밀려 들어갑니다.

사용자가 준 예시:

```text
0x1A0  8  08 00 20 4E 00 00 00 00
```

이 예시는 `speed_kmh = 100`일 때의 DBC 인코딩 형태와 맞습니다.

## 주기 송신

계기판은 메시지를 한 번 받는 것으로 끝나지 않고, 계속 주기적으로 받아야 바늘을 유지합니다.

현재 주기:

| CAN ID | 이름 | 주기 |
|---|---|---|
| `0x280` | `Motor_1` RPM | 50ms |
| `0x1A0` | `Bremse_1` Speed | 100ms |

## Timeout 동작

Board A 메시지가 끊겼는데 마지막 값을 계속 보내면 실제 상태와 계기판 표시가 달라질 수 있습니다.

그래서 마지막 정상 수신 후 1000ms가 지나면 inactive 상태로 보고 `rpm = 0`, `speed = 0`으로 송신합니다.

조건:

```text
ign_on == false
또는
마지막 Board A 메시지 수신 후 1000ms 초과
```

## 현재 구현에서 일부러 하지 않은 것

기존 구조 보존을 위해 아래 파일들은 수정하지 않았습니다.

- `Core/Src/main.c`
- `Core/Src/freertos.c`

승인 후 변경한 기존 파일은 아래 두 곳입니다.

| 파일 | 변경 이유 |
|---|---|
| `CMakeLists.txt` | `app/*.c` 빌드 구조 유지 |
| `app/gateway_tasks.c` | 기존 Gateway/Cluster task에서 브리지 함수를 호출 |

## 실차 테스트 체크리스트

CAN analyzer 또는 candump에서 아래를 확인합니다.

| 확인 항목 | 기대값 |
|---|---|
| CAN2에 `0x280` 송신 | 50ms 주기 |
| CAN2에 `0x1A0` 송신 | 100ms 주기 |
| `0x280` byte2-3 | RPM 값 변화에 따라 변경 |
| `0x1A0` byte2-3 | Speed 값 변화에 따라 변경 |
| `0x1A0` byte7 하위 4비트 | `0~15` 반복 증가 |
| Board A timeout | 1초 뒤 rpm/speed 0 송신 |
