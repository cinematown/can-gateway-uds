# Board D - Body / BCM

Board D는 도어, 방향지시등, 하이빔, 안개등 같은 Body 신호를 만들고 CAN1에 송신하는 보드입니다.

현재 연동 기준:

- IGN 수신: 기존 Board A `0x100` EngineData `byte5 bit0`
- Golf 6 IGN 수신도 지원: `0x570`/`0x572` Klemme 15 `byte0 bit1`
- Body 송신: Golf 6 DBC `BO_ 912 mGate_Komf_1`, CAN ID `0x390`, DLC 8, 100ms

## 역할

| 파일 | 역할 |
|---|---|
| `src/bcm_input.c` | DIP 스위치 / 버튼 GPIO 입력 |
| `src/bcm_signal.c` | Golf 6 `mGate_Komf_1`(`0x390`) 신호 패킹 |
| `src/bcm_can.c` | `common/can_bsp.c/h`를 통한 CAN 송수신 |
| `src/bcm_main.c` | FreeRTOS Task 연결 |

## CAN 메시지

| ID | 방향 | 내용 |
|---|---|---|
| `0x100` | Board A -> Board D | 현재 프로젝트 EngineData, IGN = byte5 bit0 |
| `0x570` | Golf 6 -> Board D | `mBSG_Last`, Klemme 15 = byte0 bit1 |
| `0x572` | Golf 6 -> Board D | `mZAS_1`, Klemme 15 = byte0 bit1 |
| `0x390` | Board D -> CAN1 | Golf 6 `mGate_Komf_1`: turn, door, high beam, fog |

## Golf 6 mGate_Komf_1 Mapping

`docs/Golf_6_PQ35.dbc` 기준:

| Signal | Bit | Board D input |
|---|---:|---|
| `GK1_Sta_Tuerkont` | 4 | any door open |
| `GK1_Fa_Tuerkont` | 16 | any door open |
| `GK1_Blinker_li` | 34 | left turn blink state |
| `GK1_Blinker_re` | 35 | right turn blink state |
| `GK1_LS1_Fernlicht` | 37 | high beam |
| `GK1_Fernlicht` | 49 | high beam |
| `GK1_Nebel_ein` | 58 | fog light |

`GK1_SleepAckn`, `GK1_Rueckfahr`, `GK1_Warnblk_Status`는 DBC 기본 상태 유지를 위해 1로 송신합니다.

## 기본 입력 매핑

입력은 pull-up 기준입니다. 스위치/버튼이 GND로 연결되면 active입니다.

| 기능 | F429ZI 기본 핀 |
|---|---|
| FL Door | PE2 |
| FR Door | PE3 |
| RL Door | PE4 |
| RR Door | PE5 |
| Left Turn Button | PE6 |
| Right Turn Button | PF6 |
| High Beam | PF7 |
| Fog Light | PF8 |

## CAN 테스트

1. Board A가 `0x100`을 보내고 `byte5 bit0 = 1`이면 Board D가 IGN ON으로 인식합니다.
2. Board D는 IGN ON 동안 Golf 6 `0x390`을 100ms 주기로 송신합니다.
3. CAN monitor에서 `0x390`이 보이고, 입력 변화에 따라 위 bit들이 변하는지 확인합니다.

주의: 현재 `board_b_gateway` 코드는 `0x100`만 CAN2로 포워딩합니다. `board_b_gateway`를 수정하지 않는 조건이면 `0x390`은 CAN1에서는 보이지만 CAN2/계기판 쪽으로는 자동 포워딩되지 않습니다.
