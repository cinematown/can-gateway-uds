# UDS CLI 테스트 가이드

보드C 의 UART CLI 를 사용해 UDS 요청을 송신하고 응답을 확인하는 방법.

## 연결
- 보드C USART3 → PC (ST-Link VCP, 115200 8N1)
- 터미널: PuTTY, minicom, CoolTerm, STM32CubeIDE Serial Monitor 등

## 기본 명령어

### `read <item>`
| 입력 | 기대 응답 |
|---|---|
| `read vin` | VIN DID `0xF190` 요청 |
| `read rpm` | RPM DID `0xF40C` 요청 |
| `read speed` | Speed DID `0xF40D` 요청 |
| `read temp` | Coolant DID `0xF40E` 요청 |

### 예시 세션
```
CLI> read rpm
[TX] 714  03 22 F4 0C 00 00 00 00
[RX] 77E  05 62 F4 0C 0D AC 00 00
RESULT: DID=0xF40C RPM=3500

CLI> read temp
[TX] 714  03 22 F4 0E 00 00 00 00
[RX] 77E  04 62 F4 0E 50 00 00 00
RESULT: DID=0xF40E Coolant=80°C
```

## 트러블슈팅

### "응답이 안 와요"
- 보드B(Gateway) 가 CAN2에 연결되어 있는지 확인 (UDS 요청은 CAN2를 흘러감)
- 120Ω 종단저항이 버스 양끝에 있는지 확인
- `candump can0` 으로 CAN2 실제 트래픽 확인

### "CLI 가 입력을 안 받아요"
- USART3 RX 인터럽트 활성화 확인 (CubeMX)
- 터미널 Local Echo 설정 확인

### "RPM 값이 이상해요"
- 보드A 페달 조작 중인지 확인
- Gateway 가 CAN2로 포워딩하고 있는지 확인 (`[GW-LOG]` 출력)
- 바이트 오더: Big-Endian (UDS) vs Little-Endian (VW 0x280) 혼동 주의
