# 보드C - UDS Diagnostic Server

**담당**: ④ 은빈
**보드**: STM32 Nucleo-F446RE (또는 F103RB)
**CAN 포트**: CAN2 × 1 (Diagnostic bus)

## 역할
- UDS ISO 14229 서버 (SID 0x22 ReadDataByIdentifier)
- UART CLI로 PC에서 테스터 역할 대체
- NRC 처리 (0x13, 0x31)

## CubeMX 설정 (Day 1)
- [ ] CAN1 (이 보드 입장에서는 Diagnostic 버스): 500kbps
  - ⚠️ F446RE 단일 보드면 CAN1을 쓰면 됨 (CAN2는 안 써도 무방)
  - F103RB의 경우 CAN만 1개
- [ ] USART2: 115200 (CLI)
- [ ] FreeRTOS 활성화
- [ ] Task: `UdsTask`, `UdsCliTask`

## CLI 사용 예시
```
> read_did F190
[TX] 7DF  03 22 F1 90
[RX] 7E8  05 62 F1 90 4B 52  → VIN(partial): KR..

> read_did F40C
[TX] 7DF  03 22 F4 0C
[RX] 7E8  05 62 F4 0C 0D AC  → RPM = 3500

> read_did 9999
[TX] 7DF  03 22 99 99
[RX] 7E8  03 7F 22 31  → NRC 0x31 (DID not found)
```

## 핀맵 (예시)
| 기능 | 핀 | 비고 |
|---|---|---|
| CAN_RX | PA11 | TJA1050 |
| CAN_TX | PA12 | TJA1050 |
| USART2_TX | PA2 | CLI 출력 |
| USART2_RX | PA3 | CLI 입력 |
