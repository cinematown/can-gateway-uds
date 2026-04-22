# 보드B - Central Gateway

**담당**: ③ 지윤 (Gateway) + ⑤ 미정 (Cluster)
**보드**: STM32 Nucleo-F446RE ⚠️ **필수** (CAN1/CAN2 2포트 필요)
**CAN 포트**: CAN1 + CAN2

## 역할
- CAN1 (Powertrain) ↔ CAN2 (Diagnostic) 게이트웨이
- RPM 임계값 초과 시 경고 메시지 CAN2 송신
- VW Passat B6 계기판 keep-alive 및 경고등 제어
- 1초 주기 트래픽 통계 UART 출력

## CubeMX 설정 (Day 1)
- [ ] CAN1: 500kbps (Powertrain)
- [ ] CAN2: 500kbps (Diagnostic) — ⚠️ F446RE만 지원
- [ ] USART2: 115200 (트래픽 로거 출력)
- [ ] FreeRTOS 활성화
- [ ] Task: `GatewayTask`, `LoggerTask`, `ClusterTask`
- [ ] Queue: `can1_rx_queue` (depth=16, item=CAN_Msg_t)

## 핀맵 (예시)
| 기능 | 핀 | 비고 |
|---|---|---|
| CAN1_RX | PA11 | TJA1050 #1 |
| CAN1_TX | PA12 | TJA1050 #1 |
| CAN2_RX | PB5 | TJA1050 #2 |
| CAN2_TX | PB6 | TJA1050 #2 |
| USART2_TX | PA2 | ST-Link VCP |

## 주의
- CAN2 클럭은 CAN1에 슬레이브. 둘 다 같은 bit timing.
- 계기판 12V 전원은 별도 어댑터. CAN GND는 공통.
