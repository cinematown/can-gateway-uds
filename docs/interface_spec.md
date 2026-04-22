# Interface Specification

> 이 문서는 **6인 병렬 개발의 계약서**입니다. 각 모듈은 이 규격만 지키면
> 서로 독립적으로 개발 가능하고, 통합 시 자동으로 연결됩니다.
> 헤더 파일 변경이 필요하면 반드시 팀장 승인 후 전원 공지.

## 모듈 의존 관계

```
┌───────────────────┐
│  common/can_db.h  │ ← 모두가 include (단순 매크로/구조체, 코드 없음)
└─────────┬─────────┘
          │
┌─────────▼──────────┐
│  common/can_bsp.*  │ ← ① 성재/민진 구현, 나머지 전원 사용
└─────────┬──────────┘
          │
  ┌───────┼──────────┬──────────┬──────────┐
  │       │          │          │          │
┌─▼──┐ ┌──▼──┐  ┌────▼────┐ ┌───▼──┐  ┌────▼────┐
│eng │ │ gw  │  │ cluster │ │ uds  │  │  main   │
│ ②  │ │ ③  │  │   ⑤    │ │  ④  │  │   ⑥    │
└────┘ └─────┘  └─────────┘ └──────┘  └─────────┘
```

## 모듈별 공개 API 요약

### ① `common/can_bsp` (성재/민진)

```c
int  CAN_BSP_Init(CAN_HandleTypeDef *hcan);
void CAN_BSP_SetRxQueue(CAN_HandleTypeDef *hcan, QueueHandle_t queue);
int  CAN_BSP_AddFilter(CAN_HandleTypeDef *hcan, uint32_t std_id);
int  CAN_BSP_AddFilterAcceptAll(CAN_HandleTypeDef *hcan);
int  CAN_BSP_Send(CAN_HandleTypeDef *hcan, const CAN_Msg_t *msg);
void CAN_BSP_GetStats(CAN_HandleTypeDef *hcan, CAN_Stats_t *out_stats);
```
- **보장**: `CAN_BSP_Send`는 non-blocking, TX 큐 full이면 -1 반환.
- **보장**: RX는 HAL 콜백에서 `xQueueSendFromISR` 로 등록된 Queue에 push.
- **보장**: Queue item 크기는 `sizeof(CAN_Msg_t)` (고정).

### ② `board_a_engine/engine_sim` (윤지)

```c
void     EngSim_Init(void);
void     EngSim_Task(void *argument);
uint16_t EngSim_GetRPM(void);
uint16_t EngSim_GetSpeed(void);
int8_t   EngSim_GetCoolantTemp(void);
```
- **책임**: `EngSim_Task` 는 `PERIOD_*_MS` 주기로 CAN1에 `CAN_ID_RPM/SPEED/COOLANT` 송신.
- **보장**: ADC 값은 내부에서 `osMutex` 없이 단일 Task 소유. 외부에서는 Get* 함수로만 읽을 것.

### ③ `board_b_gateway/gateway` (지윤)

```c
void    Gateway_Init(void);
int     Gateway_AddRoute(uint32_t src_id, uint32_t dst_id);
void    Gateway_Task(void *argument);
void    Gateway_LoggerTask(void *argument);
uint8_t Gateway_IsWarningActive(void);
```
- **책임**: `can1_rx_queue` 에서 pop → 라우팅 → CAN2 TX.
- **책임**: RPM > 5500 감지 시 `CAN_ID_WARNING` 송신.
- **의존**: main.c가 `can1_rx_queue` 생성 후 `CAN_BSP_SetRxQueue(&hcan1, q)` 호출해둬야 함.

### ④ `board_c_uds/uds_server` (은빈)

```c
void UDS_Init(void);
void UDS_Task(void *argument);
void UDS_CliTask(void *argument);
void UDS_UpdateCache_RPM(uint16_t rpm);
void UDS_UpdateCache_Speed(uint16_t kmh);
void UDS_UpdateCache_Temp(int8_t celsius);
```
- **책임**: `CAN_ID_UDS_REQ (0x7DF)` 요청 → `CAN_ID_UDS_RESP (0x7E8)` 응답.
- **책임**: CAN2 에서 RPM/Speed/Temp 메시지 자동 수신하여 내부 캐시 갱신.
- **스코프**: SID 0x22만. Single Frame만 (DLC ≤ 7 데이터).

### ⑤ `board_b_gateway/cluster_can` (미정)

```c
void Cluster_Init(void);
void Cluster_SendInitSequence(void);
void Cluster_SetRPM(uint16_t rpm);
void Cluster_SetSpeed(uint16_t kmh);
void Cluster_SetCoolantTemp(int8_t celsius);
void Cluster_SetWarning(uint8_t on);
void Cluster_Task(void *argument);
```
- **책임**: VW Passat B6 포맷 메시지 CAN2 송신 (keep-alive 포함).
- **참고**: Gateway가 대부분의 메시지를 이미 CAN2에 흘리므로, Cluster는 **Warning/Airbag-off/초기화** 같은 고유 메시지만 담당.

### ⑥ `main.c` (한결)

각 보드 `main.c` 의 책임:
- CubeMX init 후 `CAN_BSP_Init(&hcan1)` (+ 보드B는 `&hcan2` 추가)
- FreeRTOS Queue 생성: `can1_rx_queue`, (보드B) `can2_rx_queue`
- `CAN_BSP_SetRxQueue()` 등록
- `CAN_BSP_AddFilter*()` 등록 (게이트웨이는 AcceptAll, 나머지는 필요한 ID만)
- 각 모듈 `_Init()` 호출
- `osThreadNew()` 로 Task 생성
- `osKernelStart()`

## 공유 자원 (Queue/Mutex)

| 자원명 | 보드 | 생성 위치 | 용도 |
|---|---|---|---|
| `can1_rx_queue` | A, B | main.c | CAN1 RX 메시지 버퍼 |
| `can2_rx_queue` | B, C | main.c | CAN2 RX 메시지 버퍼 |
| `uart_tx_mutex` | 전체 | main.c | UART 출력 동기화 (printf racing 방지) |

## Task 우선순위 규칙

```
AboveNormal  : Gateway, UDS  (실시간 응답 필요)
Normal       : EngSim, Cluster
BelowNormal  : Logger, CLI  (실시간성 없음)
```

## 이 문서 변경 프로세스

1. 제안자가 Issue 생성 (`[interface] ...` 태그)
2. 팀장 검토 + 영향받는 담당자 태그
3. PR 머지 + 전원 공지 (Slack/카톡)
4. 각자 브랜치 rebase
