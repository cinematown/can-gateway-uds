# 보드A - Engine ECU Simulator

**담당**: ② 윤지
**보드**: STM32 Nucleo-F446RE (또는 F103RB)
**CAN 포트**: CAN1 × 1

## 역할
가변저항(액셀/브레이크 페달 시뮬레이터)의 ADC 값을 읽어 RPM/Speed/Coolant 신호를 CAN1로 송신.

## CubeMX 설정 (Day 1)
- [ ] CAN1: 500kbps (Prescaler/BS1/BS2 계산 필요, 180MHz APB1=45MHz 기준)
- [ ] ADC1: Channel 0, 1 (가변저항 2개)
- [ ] USART2: 115200 (디버그 로그)
- [ ] FreeRTOS (CMSIS-RTOS v2) 활성화
- [ ] Task: `EngSimTask` (Priority: Normal, Stack: 512)

## 핀맵 (예시)
| 기능 | 핀 | 비고 |
|---|---|---|
| CAN1_RX | PA11 | TJA1050 RXD |
| CAN1_TX | PA12 | TJA1050 TXD |
| ADC1_IN0 | PA0 | 액셀 포텐셔미터 |
| ADC1_IN1 | PA1 | 브레이크 포텐셔미터 |
| USART2_TX | PA2 | ST-Link VCP |

## 빌드
STM32CubeIDE → Import → Existing Projects → `board_a_engine/`
