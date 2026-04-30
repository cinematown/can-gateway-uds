# System Architecture

## 전체 구조

```
┌──────────────────────────┐      ┌──────────────────┐
│   보드A (Engine ECU)     │      │ 보드D Body/BCM   │
│   - 가변저항 2개 ADC     │      │ - 0x390 Body TX  │
│   - RPM/Speed/Coolant TX │      │ - Door/Turn/Lamp │
│   - 0x300 IGN TX         │      │ - 0x300 IGN RX   │
└────────────┬─────────────┘      └────────┬─────────┘
             │ CAN1 (Powertrain/Body Bus, 500kbps)
             └──────────────┬──────────────┘
                            │
              ┌─────────────▼─────────────┐
              │  보드B (Central Gateway)  │
              │  - 라우팅 테이블          │
              │  - 이상 감지 (RPM>=5000)  │
              │  - 트래픽 로거 (UART)     │
              │  - VW 계기판 HMI          │
              └─────────────┬─────────────┘
                            │ CAN2 (Diagnostic/Cluster Bus, 500kbps)
                            │
              ┌─────────────┴─────────────┐
              │                           │
              ▼                           ▼
          ┌──────────┐              ┌──────────────────┐
          │ 보드C    │              │ VW Passat B6     │
          │ (UDS)    │              │ 실차 계기판       │
          │ SID 0x22 │              │ RPM/Speed/Body   │
          │ UART CLI │              │ 경고등           │
          └──────────┘              └──────────────────┘
```

## 도메인 분리

| 도메인 | 버스 | 메시지 |
|---|---|---|
| Powertrain | CAN1 | 엔진 관련 신호 (0x280 RPM, 0x1A0 Speed, 0x288 Coolant) |
| Body | CAN1/CAN2 | Body 상태 (0x390), IGN/Keepalive (0x300) |
| Diagnostic/Cluster | CAN2 | UDS (0x714/0x77E), 계기판 제어 (0x480 Warning), Body 상태 포워딩 |

## FreeRTOS Task 구조

### 보드A (Engine ECU)
| Task | Priority | Stack | 주기 |
|---|---|---|---|
| EngSimTask | Normal | 512 | 10ms tick |
| DefaultTask | Normal | 1024 | UART CLI loop |

### 보드B (Gateway)
| Task | Priority | Stack | 주기 |
|---|---|---|---|
| DefaultTask | Normal | 1024 | CAN queue pending |

### 보드C (UDS)
| Task | Priority | Stack | 주기 |
|---|---|---|---|
| DefaultTask | Normal | 1024 | UART CLI + UDS response polling |

### 보드D (Body / BCM)
| Task | Priority | Stack | 주기 |
|---|---|---|---|
| EngineSimTask slot | Low | 512 | 100ms Body status TX |
| BcmInputTask | Low | 512 | 20ms GPIO polling |
| BcmIgnRxTask | Low | 512 | CAN RX pending |

## 데이터 흐름 예시 — "페달 밟기 → 계기판 반응"

1. 사용자가 보드A 포텐셔미터 회전
2. 보드A ADC → RPM 계산 → CAN1 `0x280` 메시지 50ms 주기 송신
3. 보드B CAN1 RX 콜백 → `can1_rx_queue` push
4. `GatewayTask`가 Queue pop → 라우팅 테이블 조회
5. 이상 감지: RPM >= 5000 → `s_warning_active = 1`, RPM < 4500 → `s_warning_active = 0`
6. CAN2로 `0x280` 포워딩 → 계기판이 수신하여 바늘 움직임
7. 보드D가 도어/방향지시등/하이빔/안개등 상태를 `0x390`으로 송신
8. 보드B가 `0x390`을 CAN2로 포워딩
9. PC CLI에서 `read rpm` 입력
10. 보드C가 `0x714` 요청 → `0x77E` 응답 확인

## 확장 로드맵

- [ ] ISO-TP 멀티프레임 (VIN 전체 전송)
- [ ] UDS 0x10 세션 / 0x27 보안
- [ ] Bus-off 복구, 에러 카운터 모니터링
- [ ] CAN 부트로더 (SID 0x34/0x36/0x37)
