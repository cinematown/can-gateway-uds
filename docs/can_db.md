# CAN Database

> 이 문서는 사람용 요약. 실제 코드는 [`common/can_db/can_db.h`](../common/can_db/can_db.h) 를 사용.

## CAN1 - Powertrain Bus (500kbps)

| CAN ID | 이름 | DLC | 주기 | 송신자 | 수신자 | 인코딩 |
|---|---|---|---|---|---|---|
| 0x280 | RPM | 8 | 50ms | 보드A | 보드B | byte[2:3] = rpm × 4 (LE) |
| 0x1A0 | Speed | 8 | 100ms | 보드A | 보드B | byte[0] = 0x18 (ABS off), byte[2:3] = kmh × 100 (LE) |
| 0x288 | Coolant Temp | 8 | 1000ms | 보드A | 보드B | byte[3] = temp + 40 |

## CAN2 - Diagnostic Bus (500kbps)

| CAN ID | 이름 | DLC | 주기 | 송신자 | 수신자 | 인코딩 |
|---|---|---|---|---|---|---|
| 0x280 | RPM (포워딩) | 8 | 50ms | 보드B | 계기판, 보드C | 동일 |
| 0x1A0 | Speed (포워딩) | 8 | 100ms | 보드B | 계기판, 보드C | 동일 |
| 0x288 | Coolant Temp (포워딩) | 8 | 1000ms | 보드B | 계기판, 보드C | 동일 |
| 0x480 | Warning | 8 | 이벤트 | 보드B | 계기판 | byte[0] bitfield |
| 0x050 | Airbag off | 8 | 100ms | 보드B | 계기판 | byte[1] = 0x80 |
| 0x7DF | UDS Request | 8 | 이벤트 | 테스터(CLI) | 보드C | ISO-TP Single Frame |
| 0x7E8 | UDS Response | 8 | 이벤트 | 보드C | 테스터 | ISO-TP Single Frame |

## Warning Bitfield (0x480 byte[0])

| Bit | 의미 |
|---|---|
| 0 | RPM over threshold |
| 1 | Overheat |
| 2 | General warning |
| 3~7 | Reserved |

## Bit Timing (500kbps @ F446RE APB1=45MHz)

| 파라미터 | 값 |
|---|---|
| Prescaler | 5 |
| BS1 | 13 tq |
| BS2 | 2 tq |
| SJW | 1 |
| Sample Point | 87.5% |

> 검증 필수 — 보드별 클럭 소스에 따라 달라짐. F103RB는 APB1=36MHz 기준으로 재계산.
