# Interface Specification

> 이 문서는 **6인 병렬 개발의 계약서**입니다. 각 모듈은 이 규격만 지키면
> 서로 독립적으로 개발 가능하고, 통합 시 자동으로 연결됩니다.
> 헤더 파일 변경이 필요하면 반드시 팀장 승인 후 전원 공지.

## 모듈 의존 관계

```
┌───────────────────┐
│ common/signal_db.h│ ← DBC 기반 신호 매크로/인코더
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

### ① `common/can_bsp`

```c
HAL_StatusTypeDef CAN_BSP_Init(void);
HAL_StatusTypeDef CAN_BSP_Send(uint32_t id, uint8_t *data, uint8_t len);
HAL_StatusTypeDef CAN_BSP_SendTo(CAN_HandleTypeDef *hcan, uint32_t id, uint8_t *data, uint8_t len);
bool CAN_BSP_Read(CAN_RxMessage_t *p_msg, uint32_t timeout);
HAL_StatusTypeDef CAN_BSP_GetRxMessage(CAN_RxMessage_t *p_msg);
```
- **보장**: 보드 A/C는 CAN1 기본 송수신, 보드 B는 `BOARD_B_GATEWAY` 정의로 CAN1/CAN2를 모두 활성화.
- **보장**: RX는 HAL 콜백에서 `osMessageQueue`에 push, UDS 폴링 코드는 `CAN_BSP_GetRxMessage()` 사용 가능.

### ② `board_a_engine/engine_sim` (윤지)

```c
void EngineSim_Init(void);
void EngineSim_Task(void *argument);
void EngineSim_SetThrottle(uint8_t throttle);
void EngineSim_GetStatus(EngineSimStatus_t *status);
```
- **책임**: `EngineSim_Task`는 `0x280/0x1A0/0x288`을 DBC 포맷으로 CAN1 송신.
- **보장**: ADC 값은 내부에서 `osMutex` 없이 단일 Task 소유. 외부에서는 Get* 함수로만 읽을 것.

### ③ `board_b_gateway/gateway` (지윤)

```c
void StartDefaultTask(void *argument);
```
- **책임**: CAN1의 엔진/속도/냉각수 메시지를 CAN2로 포워딩.
- **책임**: RPM >= 5000 감지 시 `0x480` warning 송신.
- **책임**: CAN2의 UDS request `0x714`를 CAN1로 라우팅.

### ④ `board_c_uds/uds_server` (은빈)

```c
void UDS_Server_Init(void);
void UDS_Server_Process(void);
void UDS_Execute_Diagnostic(uint16_t did, const char *label);
```
- **책임**: UART CLI `read vin|rpm|speed|temp` → UDS SID 0x22 요청.
- **책임**: `0x714` 요청 송신, `0x77E` 응답 수신.

### ⑤ `cluster_can` (추가 예정)

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

각 보드 `main.c`는 CubeMX 초기화와 FreeRTOS 시작만 담당합니다.
실제 앱 진입점은 각 보드 `src/*_main.c`의 `StartDefaultTask()` 오버라이드입니다.

## 공유 자원 (Queue/Mutex)

| 자원명 | 보드 | 생성 위치 | 용도 |
|---|---|---|---|
| `can_rx_q` | 전체 | `common/can_bsp.c` | CAN RX 메시지 버퍼 |
| `uart_rx_q` | 전체 | `common/uart.c` | UART RX 버퍼 |
| `uart_tx_mutex` | 전체 | `common/uart.c` | UART 출력 동기화 |

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
