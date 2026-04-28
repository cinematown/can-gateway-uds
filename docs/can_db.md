# CAN Database

> 이 문서는 사람용 요약. 실제 코드는 [`common/signal_db.h`](../common/signal_db.h)를 사용.
> 원본 DBC는 [`docs/VW_Passat_B6.dbc`](VW_Passat_B6.dbc)입니다.

## CAN1 - Powertrain Bus (500kbps)

| CAN ID | 이름 | DLC | 주기 | 송신자 | 수신자 | 인코딩 |
|---|---|---|---|---|---|---|
| 0x280 | Motor_1 / RPM | 8 | 50ms | 보드A | 보드B | `Motordrehzahl`, start 16, len 16, scale 0.25 |
| 0x1A0 | Bremse_1 / Speed | 8 | 100ms | 보드A | 보드B | `BR1_Rad_kmh`, start 17, len 15, scale 0.01 |
| 0x288 | Motor_2 / Coolant | 8 | 1000ms | 보드A | 보드B | `MO2_Kuehlm_T`, start 8, len 8, scale 0.75, offset -48 |

## CAN2 - Diagnostic Bus (500kbps)

| CAN ID | 이름 | DLC | 주기 | 송신자 | 수신자 | 인코딩 |
|---|---|---|---|---|---|---|
| 0x280 | RPM (포워딩) | 8 | 50ms | 보드B | 계기판, 보드C | 동일 |
| 0x1A0 | Speed (포워딩) | 8 | 100ms | 보드B | 계기판, 보드C | 동일 |
| 0x288 | Coolant Temp (포워딩) | 8 | 1000ms | 보드B | 계기판, 보드C | 동일 |
| 0x480 | Warning | 8 | 이벤트 | 보드B | 계기판 | byte[0] bitfield |
| 0x050 | Airbag off | 8 | 100ms | 보드B | 계기판 | byte[1] = 0x80 |
| 0x714 | UDS Request | 8 | 이벤트 | 보드C CLI | 게이트웨이/타깃 | ISO-TP Single Frame |
| 0x77E | UDS Response | 8 | 이벤트 | 게이트웨이/타깃 | 보드C CLI | ISO-TP Single Frame |

## Warning Bitfield (0x480 byte[0])

| Bit | 의미 |
|---|---|
| 0 | RPM over threshold |
| 1 | Overheat |
| 2 | General warning |
| 3~7 | Reserved |

## Bit Timing (500kbps @ STM32F429ZI)

| 파라미터 | 값 |
|---|---|
| Prescaler | 9 |
| BS1 | 4 tq 또는 8 tq |
| BS2 | 5 tq 또는 1 tq |
| SJW | 1 |
| Baudrate | 500kbps |

> 보드 A/B/C 모두 STM32F429ZI 기준이며, 보드 C는 리팩터링된 UDS CubeMX 설정을 기준으로 합니다.
