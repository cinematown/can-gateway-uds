# Board D - Body / BCM

Board D는 도어, 방향지시등, 하이빔, 안개등 같은 Body 신호를 만들고
`0x470` CAN 메시지로 Gateway에 보내는 보드입니다.

## 역할

| 파일 | 역할 |
|---|---|
| `src/bcm_input.c` | DIP 스위치, 버튼 GPIO 입력 |
| `src/bcm_signal.c` | `protocol_ids.h` 매크로로 `0x470` 데이터 조립 |
| `src/bcm_can.c` | `common/can_bsp.c/h`를 통한 CAN 송수신 |
| `src/bcm_main.c` | FreeRTOS Task 연결 |

## 다른 보드와의 관계

```text
Board A Engine ECU
  - 0x300 IGN/keepalive status 송신
  - 0x280/0x1A0/0x288 엔진/속도/냉각수 송신

Board D Body / BCM
  - 0x300 IGN status 수신
  - 0x470 Body status 송신

Board B Gateway
  - Board A, Board D 메시지 수신
  - 계기판/진단 버스 방향으로 필요한 메시지 포워딩

Board C UDS
  - 진단 요청/응답 담당
  - Gateway 또는 진단 CAN 라인과 연결
```

## CAN 메시지

| ID | 방향 | 내용 |
|---|---|---|
| `0x300` | Board A -> Board D | IGN ON/OFF, byte0 bit0 |
| `0x470` | Board D -> Board B | 방향지시등, 도어, 하이빔, 안개등 |

`0x470` 비트 위치는 직접 하드코딩하지 않고
`common/protocol_ids.h`의 `VW470_SET_*` 매크로를 사용합니다.

## 기본 입력 매핑

입력은 pull-up 기준입니다. 스위치/버튼이 GND로 떨어질 때 active입니다.

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

## 빌드

단독 송신 테스트:

```sh
cmake --preset Debug
cmake --build --preset Debug
```

Engine ECU의 IGN ON을 기다리는 통합 테스트:

```sh
cmake --preset Debug -DBCM_BODY_WAIT_FOR_IGN=1
cmake --build --preset Debug
```

## CAN 테스트

1. Board A engine 쪽에서 `0x300` 프레임의 byte0 bit0를 1로 보내면 Board D가 IGN ON으로 인식합니다.
2. Board D는 `0x470` 프레임을 송신하고, Gateway에서 해당 ID가 보이는지 확인합니다.
3. 벤치에서는 `BOARD_D_BODY` 루트에서 `cmake --preset Debug` 후 빌드하고, 시리얼 로그로 `IGN ON/OFF`와 `Body module ready` 메시지를 확인합니다.
