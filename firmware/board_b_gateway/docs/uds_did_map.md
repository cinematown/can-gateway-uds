# UDS DID Map

SID 0x22 (ReadDataByIdentifier) 지원 DID 목록.

## 지원 DID

| DID (hex) | 이름 | 길이 (bytes) | 설명 | 단위 |
|---|---|---|---|---|
| 0xF190 | VIN | 17 | Vehicle Identification Number | ASCII |
| 0xF40C | Engine RPM | 2 | 현재 엔진 회전수 | rpm, Big-Endian |
| 0xF40D | Vehicle Speed | 2 | 현재 차량 속도 | km/h, Big-Endian |
| 0xF40E | Coolant Temp | 1 | 냉각수 온도 | °C, signed |

> ⚠️ DID 0xF190 (VIN)은 17바이트라 ISO-TP 멀티프레임이 필요함.
> 2주 스코프에서는 앞 4바이트만 Single Frame으로 반환 (또는 NRC 0x31).
> 면접에서 "풀 VIN 전송은 ISO-TP 확장 스프린트에서 구현 예정" 으로 어필.

## 요청/응답 프레임 예시

### Positive Response (RPM = 3500)
```
Request:  714  03 22 F4 0C 00 00 00 00
                │  │  └──┬──┘
                │  │     DID (0xF40C)
                │  └─ SID (0x22)
                └──── PCI length (3)

Response: 77E  05 62 F4 0C 0D AC 00 00
                │  │  └──┬──┘  └──┬──┘
                │  │     DID      Value (0x0DAC = 3500 rpm)
                │  └─ SID + 0x40 (0x62)
                └──── PCI length (5)
```

### Negative Response (존재하지 않는 DID)
```
Request:  714  03 22 99 99 00 00 00 00

Response: 77E  03 7F 22 31 00 00 00 00
                │  │  │  └─ NRC 0x31 (Request Out of Range)
                │  │  └──── Original SID
                │  └─────── Negative Response SID (0x7F)
                └────────── PCI length (3)
```

## NRC 테이블

| NRC | 의미 | 발생 조건 |
|---|---|---|
| 0x11 | Service Not Supported | 지원 안 하는 SID (0x10, 0x27 등) |
| 0x13 | Incorrect Message Length | PCI length 불일치 |
| 0x31 | Request Out of Range | DID 테이블에 없음 |
