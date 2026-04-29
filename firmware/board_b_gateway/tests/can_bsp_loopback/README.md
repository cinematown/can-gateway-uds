# CAN BSP Loopback Test

①번 팀원(성재/민진)이 CAN 드라이버 구현 후 검증하는 방법.

## Loopback Mode 테스트 (CAN 트랜시버 없이)

CubeMX 에서 CAN 모드를 `Loopback` 으로 바꾸고 빌드.

```c
/* 자기가 보낸 메시지를 자기가 수신하는지 확인 */
CAN_Msg_t tx = { .id = 0x123, .dlc = 3, .data = {0xDE, 0xAD, 0xBE} };
CAN_BSP_Send(&hcan1, &tx);

/* RX Queue 에 똑같은 메시지가 들어오는지 */
CAN_Msg_t rx;
if (xQueueReceive(can1_rx_queue, &rx, 100) == pdTRUE) {
    // rx.id == 0x123 && rx.data[0] == 0xDE ...
}
```

## Normal Mode 테스트 (실제 CAN 통신)

보드 2대 + TJA1050 2개 + 120Ω 종단저항 2개 필요.

### 배선
```
[Nucleo-A] ─── [TJA1050-A] ═══╗
                               ║  CAN_H/CAN_L (twisted pair)
                   120Ω ─┐    ║    ┌─ 120Ω
                         ║    ║    ║
[Nucleo-B] ─── [TJA1050-B] ═══╝
```
- Nucleo PA12 → TJA1050 TXD
- Nucleo PA11 ← TJA1050 RXD
- TJA1050 Vcc = 5V (Nucleo 5V 핀)
- TJA1050 STBY = GND (Normal mode)
- CAN_H ↔ CAN_H, CAN_L ↔ CAN_L

### 테스트 시퀀스
1. 보드A: 1초마다 `0x100` 송신
2. 보드B: `0x100` 수신 → UART 로 "[RX] 0x100 ..." 출력
3. 둘 다 `CAN_BSP_GetStats()` 로 tx_count/rx_count 매칭 확인

### 실패 시 체크리스트
- [ ] bit timing (Prescaler/BS1/BS2) 값이 두 보드 동일
- [ ] 500kbps Sample point 87.5% 근처
- [ ] 종단저항 120Ω 양끝에 모두 있음
- [ ] CAN_H/L 스와프 아닌지
- [ ] GND 공통 연결
- [ ] TJA1050 STBY 핀 GND 에 묶었는지
