# Board A Engine ECU Simulator

`board_a_engine`은 STM32F429 기반 Engine ECU 시뮬레이터 펌웨어입니다. 스로틀/브레이크 입력을 받아 간단한 엔진 모델을 계산하고, RPM/속도/냉각수 온도/상태 값을 CAN 메시지로 주기 송신합니다. USART3 CLI와 Windows용 Qt GUI를 함께 제공해 하드웨어 ADC 입력 또는 PC 제어 입력으로 시뮬레이션을 운용할 수 있습니다.

## 주요 기능

- ADC 입력 기반 스로틀/브레이크 제어
- UART CLI 기반 수동 제어 및 상태 조회
- Qt Widgets 기반 EngineSim Control Panel 제공
- FreeRTOS 태스크 기반 엔진 모델 갱신
- CAN1 500 kbps로 엔진 데이터 송신
- 모니터링용 고정 포맷 텔레메트리 출력

## 대상 하드웨어

- MCU: STM32F429ZI 계열
- UART: USART3, ST-LINK VCP, `115200 8-N-1`
- CAN: CAN1, Normal mode, 500 kbps
- ADC:
  - Throttle: PA0 / ADC1_IN0
  - Brake: PA3 / ADC1_IN3

주요 핀 설정은 `stm32f429zi_cli.ioc`와 CubeMX 생성 소스(`Core/Src/*.c`)를 기준으로 합니다.

## 프로젝트 구조

```text
.
|-- Core/                       # STM32CubeMX 생성 HAL/FreeRTOS 소스
|-- Drivers/                    # CMSIS, STM32F4 HAL 드라이버
|-- Middlewares/Third_Party/    # FreeRTOS
|-- MyApp/                      # 사용자 애플리케이션 코드
|   |-- engine_sim.c            # 엔진 모델, ADC 입력 처리, CAN 송신
|   |-- cli.c / cli_cmd.c       # UART CLI 및 EngineSim 명령
|   |-- can_bsp.c               # CAN 초기화/송수신 BSP
|   |-- adc_driver.c            # 스로틀/브레이크 ADC 읽기
|   `-- uart.c                  # USART3 기반 CLI 입출력
|-- GUI/engine_sim/             # Windows Qt Widgets 제어 GUI
|-- cmake/                      # CMake toolchain 및 CubeMX CMake 설정
|-- CMakeLists.txt              # 펌웨어 빌드 설정
`-- CMakePresets.json           # Debug/Release preset
```

## 동작 흐름

1. `main()`에서 HAL, GPIO, USART3, CAN1, ADC1, FreeRTOS를 초기화합니다.
2. `defaultTask`는 `StartDefaultTask()`로 진입해 UART, CLI, EngineSim, CAN BSP를 초기화합니다.
3. `EngSimTask`는 `EngineSim_Task()`를 실행합니다.
4. EngineSim은 50 ms 주기로 엔진 모델을 갱신하고 CAN 엔진 데이터를 송신합니다.
5. CLI의 `monitor` 기능이 켜져 있으면 GUI/로그 파싱용 상태 라인을 주기 출력합니다.

## 빌드

필요 도구:

- CMake 3.22 이상
- Ninja
- `arm-none-eabi-gcc` toolchain
- STM32CubeIDE 또는 STM32CubeProgrammer

Debug 빌드:

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

Release 빌드:

```powershell
cmake --preset Release
cmake --build --preset Release
```

빌드 산출물은 preset에 따라 `build/Debug/` 또는 `build/Release/` 아래에 생성됩니다.

## 펌웨어 사용

보드에 펌웨어를 다운로드한 뒤 ST-LINK VCP COM 포트에 `115200 8-N-1`로 접속하면 CLI가 표시됩니다.

부팅 메시지:

```text
==================================
   Engine ECU Simulator
   Build: <date> <time>
   Type 'help'
==================================

CLI >
```

기본 명령:

| 명령 | 설명 |
| --- | --- |
| `help` | 등록된 CLI 명령 목록 표시 |
| `sim_help` | EngineSim 명령 도움말 표시 |
| `mode adc` | ADC 입력 모드로 전환 |
| `mode uart` | UART/GUI 입력 모드로 전환 |
| `throttle <0~100>` | 스로틀 값을 설정하고 UART 모드로 전환 |
| `brake <0~100>` | 브레이크 값을 설정하고 UART 모드로 전환 |
| `pedal <throttle> <brake>` | 스로틀/브레이크를 동시에 설정 |
| `stop` | 스로틀/브레이크를 0으로 초기화 |
| `status` | 현재 EngineSim 상태 출력 |
| `monitor on [interval_ms]` | 상태 라인을 주기 출력, 최소 100 ms |
| `monitor off` | 주기 출력 중지 |
| `monitor once` | 상태 라인 1회 출력 |
| `sim_reset` | EngineSim 상태 초기화 |

모니터 출력 예시:

```text
MON mode=uart throttle=50 brake=0 rpm=3200 speed=60 coolant=102 tx=1234
```

## CAN 프로토콜

EngineSim은 CAN ID `0x100`으로 8바이트 엔진 데이터를 송신합니다.

| Byte | Field | Type | 설명 |
| --- | --- | --- | --- |
| 0-1 | RPM | `uint16_t`, little-endian | 엔진 RPM |
| 2-3 | Speed | `uint16_t`, little-endian | 차량 속도 |
| 4 | Coolant | `uint8_t` | 냉각수 온도 |
| 5 | Status | `uint8_t` | bit0: IGN ON, bit1~7: Alive Counter |
| 6 | Reserved | `uint8_t` | 예약 |
| 7 | Reserved | `uint8_t` | 예약 |

관련 정의는 `MyApp/protocol_ids.h`에 있습니다.

## GUI 사용

`GUI/engine_sim`에는 Windows용 Qt Widgets 제어 패널이 있습니다. GUI는 Qt SerialPort 모듈 없이 Win32 COM 포트를 직접 사용합니다.

추천 실행 방법:

1. Qt Creator에서 `GUI/engine_sim/CMakeLists.txt`를 엽니다.
2. Desktop Qt kit를 선택합니다.
3. `engine_sim` 타깃을 빌드/실행합니다.
4. 보드의 ST-LINK VCP COM 포트를 선택하고 baudrate `115200`으로 연결합니다.

연결되면 GUI가 자동으로 `monitor on <interval_ms>`와 `status`를 전송해 상태 표시를 갱신합니다. `Live update`가 켜져 있으면 스로틀/브레이크 컨트롤 변경이 `pedal <throttle> <brake>` 명령으로 전송됩니다.

자세한 내용은 `GUI/engine_sim/README.md`를 참고하세요.

## 개발 메모

- CubeMX 재생성 시 `Core/`, `Drivers/`, `Middlewares/`, `cmake/stm32cubemx/` 변경 사항을 확인하세요.
- 사용자 로직은 가능한 `MyApp/` 아래에 유지하는 것이 좋습니다.
- CLI/GUI 파싱 안정성을 위해 `MON ... key=value` 출력 포맷은 변경 시 GUI 파서도 함께 수정해야 합니다.
- CAN payload 변경 시 `MyApp/protocol_ids.h`와 수신 보드의 프로토콜 정의를 함께 갱신하세요.
