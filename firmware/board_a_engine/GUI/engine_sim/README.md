# EngineSim GUI

`GUI/engine_sim`은 `board_a_engine` 펌웨어의 UART CLI를 제어하기 위한 Windows용 Qt Widgets 앱입니다. COM 포트에 연결해 스로틀/브레이크 값을 조작하고, 펌웨어가 출력하는 모니터 라인을 파싱해 RPM/속도/냉각수 온도/CAN 송신 카운트를 표시합니다.

## 요구 사항

- Windows
- Qt 5 또는 Qt 6 Widgets
- C++17 지원 컴파일러
- 보드 ST-LINK VCP 연결

이 앱은 Windows COM 포트를 Win32 API로 직접 사용하므로 Qt SerialPort 모듈이 필요 없습니다.

## Qt Creator에서 실행

권장 방법:

1. Qt Creator에서 `GUI/engine_sim/CMakeLists.txt`를 엽니다.
2. Desktop Qt kit를 선택합니다.
3. `engine_sim` 타깃을 빌드하고 실행합니다.

Qt Creator가 이전 설정을 캐시하고 있다면 기존 Qt Creator 빌드 디렉터리를 삭제하거나 CMake configure를 다시 실행하세요.

선택 사항으로 qmake 프로젝트도 제공합니다.

```text
GUI/engine_sim/engine_sim.pro
```

## 연결 설정

펌웨어 CLI는 USART3/ST-LINK VCP를 사용합니다.

```text
Baudrate : 115200
Data     : 8 bit
Parity   : None
Stop     : 1 bit
Flow     : None
```

GUI에서 COM 포트를 선택한 뒤 `Connect`를 누르면 자동으로 모니터링을 시작합니다.

## 사용 CLI 명령

GUI는 펌웨어의 기존 CLI 명령을 그대로 사용합니다.

| 명령 | GUI에서의 용도 |
| --- | --- |
| `mode adc` | ADC 입력 모드 전환 |
| `mode uart` | UART/GUI 입력 모드 전환 |
| `pedal <throttle> <brake>` | 스로틀/브레이크 값 전송 |
| `stop` | 입력값 정지 |
| `sim_reset` | 시뮬레이터 상태 초기화 |
| `status` | 현재 상태 조회 |
| `monitor on <interval_ms>` | 상태 주기 수신 시작 |
| `monitor off` | 상태 주기 수신 중지 |
| `monitor once` | 상태 1회 수신 |

연결 직후 GUI는 `monitor on <interval_ms>`를 자동 전송하고, 잠시 뒤 `status`를 요청합니다.

## 모니터 라인 포맷

GUI는 아래처럼 `MON`으로 시작하는 한 줄 텔레메트리를 파싱합니다.

```text
MON mode=uart throttle=50 brake=0 rpm=3200 speed=60 coolant=102 tx=1234
```

파싱 키:

- `mode`: `adc` 또는 `uart`
- `throttle`: 스로틀 퍼센트
- `brake`: 브레이크 퍼센트
- `rpm`: 엔진 RPM
- `speed`: 차량 속도
- `coolant`: 냉각수 온도
- `tx`: CAN 송신 카운트

펌웨어 쪽 출력 포맷을 바꾸면 `mainwindow.cpp`의 모니터 라인 파서도 함께 수정해야 합니다.

## Live Update

`Live update`가 켜져 있으면 스로틀/브레이크 슬라이더 변경 시 `pedal <throttle> <brake>` 명령을 짧게 디바운스해 전송합니다. 값이 전송되면 펌웨어는 자동으로 UART 모드로 전환됩니다.
