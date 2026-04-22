# System Architecture

## 전체 구조

```
┌──────────────────────────┐
│   보드A (Engine ECU)     │
│   - 가변저항 2개 ADC     │
│   - RPM/Speed/Coolant TX │
└────────────┬─────────────┘
             │ CAN1 (Powertrain Bus, 500kbps)
             │
┌────────────▼─────────────┐
│  보드B (Central Gateway) │
│  - 라우팅 테이블         │
│  - 이상 감지 (RPM>5500)  │
│  - 트래픽 로거 (UART)    │
│  - VW 계기판 HMI         │◄── CAN2로 계기판 직결
└────┬───────────────┬─────┘
     │               │
     │ CAN2 (Diagnostic Bus, 500kbps)
     │               │
     ▼               ▼
┌──────────┐   ┌──────────────────┐
│ 보드C    │   │ VW Passat B6     │
│ (UDS)    │   │ 실차 계기판       │
│ SID 0x22 │   │ RPM/Speed 바늘   │
│ UART CLI │   │ 경고등           │
└──────────┘   └──────────────────┘
```

## 도메인 분리

| 도메인 | 버스 | 메시지 |
|---|---|---|
| Powertrain | CAN1 | 엔진 관련 신호 (0x280 RPM, 0x1A0 Speed, 0x288 Coolant) |
| Diagnostic/Body | CAN2 | UDS (0x7DF/0x7E8), 계기판 제어 (0x480 Warning) |

## FreeRTOS Task 구조

### 보드A (Engine ECU)
| Task | Priority | Stack | 주기 |
|---|---|---|---|
| EngSimTask | Normal | 512 | 10ms tick |

### 보드B (Gateway)
| Task | Priority | Stack | 주기 |
|---|---|---|---|
| GatewayTask | AboveNormal | 512 | Queue pending |
| ClusterTask | Normal | 512 | 10ms tick |
| LoggerTask | BelowNormal | 512 | 1000ms |

### 보드C (UDS)
| Task | Priority | Stack | 주기 |
|---|---|---|---|
| UdsTask | AboveNormal | 512 | Queue pending |
| UdsCliTask | Normal | 512 | UART line pending |

## 데이터 흐름 예시 — "페달 밟기 → 계기판 반응"

1. 사용자가 보드A 포텐셔미터 회전
2. 보드A ADC → RPM 계산 → CAN1 `0x280` 메시지 50ms 주기 송신
3. 보드B CAN1 RX 콜백 → `can1_rx_queue` push
4. `GatewayTask`가 Queue pop → 라우팅 테이블 조회
5. 이상 감지: RPM > 5500 → `s_warning_active = 1`, CAN2 `0x480` 송신
6. CAN2로 `0x280` 포워딩 → 계기판이 수신하여 바늘 움직임
7. 보드C가 CAN2 `0x280`을 수신하면 내부 캐시(`s_cache_rpm`) 갱신
8. PC CLI에서 `read_did F40C` → UDS 요청 → 현재 RPM 응답

## 확장 로드맵

- [ ] ISO-TP 멀티프레임 (VIN 전체 전송)
- [ ] UDS 0x10 세션 / 0x27 보안
- [ ] Bus-off 복구, 에러 카운터 모니터링
- [ ] CAN 부트로더 (SID 0x34/0x36/0x37)
