# Contributing Guide

이 문서는 Windows 개발 환경을 기준으로 작성합니다. 현재 펌웨어는 모든 보드가 STM32F429ZI 기준이며, 각 보드는 `firmware/board_*` 아래에서 독립적으로 CMake 빌드됩니다.

## 1. 저장소 구조

```text
can-gateway-uds/
├── common/                     # 전 보드 공통 코드
│   ├── can_bsp.c/h
│   ├── cli.c/h
│   ├── protocol_ids.h
│   ├── signal_db.h
│   └── uart.c/h
├── firmware/
│   ├── board_a_engine/         # 보드 A 단위 빌드 프로젝트
│   ├── board_b_gateway/        # 보드 B 단위 빌드 프로젝트
│   ├── board_c_uds/            # 보드 C 단위 빌드 프로젝트
│   └── board_d_body/           # 보드 D 단위 빌드 프로젝트
├── docs/
├── tools/
└── CMakeLists.txt              # 통합 빌드 관리용
```

각 보드 폴더에는 `CMakeLists.txt`, `CMakePresets.json`, `.ioc`, `Core/`, `Drivers/`, `Middlewares/`, `src/`가 있어야 합니다. 앱 전용 코드는 `src/`에 두고, `Core/`는 CubeMX 생성 코드 중심으로 유지합니다.

## 2. Windows 로컬 빌드 준비

### 필수 도구

Windows PowerShell에서 아래 명령이 모두 잡혀야 합니다.

```powershell
git --version
cmake --version
make --version
arm-none-eabi-gcc --version
arm-none-eabi-g++ --version
```

권장 설치:

| 도구 | 용도 |
|---|---|
| Git for Windows | 저장소 clone, branch 작업 |
| CMake 3.22 이상 | 보드별/통합 빌드 |
| STM32CubeCLT 또는 Arm GNU Toolchain | `arm-none-eabi-gcc`, `arm-none-eabi-g++`, `make` |
| STM32CubeIDE | `.ioc` 확인, 디버깅, 플래시 |

도구가 설치되어 있는데 `where arm-none-eabi-gcc`가 실패하면, GNU Arm toolchain의 `bin` 폴더와 `make`가 있는 폴더를 Windows `PATH`에 추가하세요.

예시:

```powershell
$env:Path += ";C:\ST\STM32CubeCLT\GNU-tools-for-STM32\bin"
$env:Path += ";C:\ST\STM32CubeCLT\CMake\bin"
$env:Path += ";C:\ST\STM32CubeCLT\Ninja\bin"
```

설치 경로는 PC마다 다를 수 있으니 실제 폴더명을 확인해서 맞춰야 합니다.

## 3. 첫 클론

```powershell
git clone https://github.com/<OWNER>/can-gateway-uds.git
cd can-gateway-uds
git checkout feature/<your-module>
```

STM32CubeIDE에서 열 때는 루트가 아니라 담당 보드 폴더를 Import합니다.

예시:

```text
firmware/board_a_engine
firmware/board_b_gateway
firmware/board_c_uds
firmware/board_d_body
```

## 4. 보드별 단위 빌드

단위 빌드는 담당 보드만 configure/build합니다. PR 전에 최소한 자기 담당 보드는 반드시 성공해야 합니다.

### Board A - Engine

```powershell
cmake --preset Debug --fresh -S firmware/board_a_engine
cmake --build firmware/board_a_engine/build/Debug --parallel
```

### Board B - Gateway

```powershell
cmake --preset Debug --fresh -S firmware/board_b_gateway
cmake --build firmware/board_b_gateway/build/Debug --parallel
```

### Board C - UDS

```powershell
cmake --preset Debug --fresh -S firmware/board_c_uds
cmake --build firmware/board_c_uds/build/Debug --parallel
```

### Board D - Body / BCM

```powershell
cmake --preset Debug --fresh -S firmware/board_d_body
cmake --build firmware/board_d_body/build/Debug --parallel
```

생성물은 각 보드의 `build/Debug` 아래에 생깁니다.

예시:

```text
firmware/board_a_engine/build/Debug/board_a_engine.elf
firmware/board_b_gateway/build/Debug/board_b_gateway.elf
firmware/board_c_uds/build/Debug/board_c_uds.elf
firmware/board_d_body/build/Debug/board_d_body.elf
```

## 5. 통합 빌드

통합 빌드는 루트 `CMakeLists.txt`에서 네 보드를 모두 configure/build합니다. `common/`을 수정했거나 여러 보드 간 인터페이스를 건드렸다면 반드시 통합 빌드를 돌립니다.

```powershell
cmake --preset Debug --fresh
cmake --build --preset Debug --parallel
```

특정 타깃만 직접 호출할 수도 있습니다.

```powershell
cmake --build build/Debug --target build_board_a_engine --parallel
cmake --build build/Debug --target build_board_b_gateway --parallel
cmake --build build/Debug --target build_board_c_uds --parallel
cmake --build build/Debug --target build_board_d_body --parallel
cmake --build build/Debug --target build_firmware --parallel
```

## 6. 클린 빌드

CMake 캐시가 꼬였거나 `.ioc`/toolchain 경로를 바꾼 뒤에는 빌드 폴더를 지우고 다시 configure합니다.

```powershell
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force firmware/board_a_engine/build -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force firmware/board_b_gateway/build -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force firmware/board_c_uds/build -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force firmware/board_d_body/build -ErrorAction SilentlyContinue

cmake --preset Debug --fresh
cmake --build --preset Debug --parallel
```

## 7. GitHub Actions 구성

GitHub에 올리면 `.github/workflows/firmware-ci.yml`이 PR과 `main` push에서 자동 실행됩니다.

구성 의도:

| Job | 목적 |
|---|---|
| `layout` | `common/`, `docs/`, `firmware/board_*` 구조가 맞는지 빠르게 확인 |
| `unit-build` | 보드 A/B/C/D를 matrix로 각각 단위 빌드 |
| `integration-build` | 루트 `build_firmware`로 전체 통합 빌드 |

CI는 Ubuntu runner에서 `gcc-arm-none-eabi`, `binutils-arm-none-eabi`, `libnewlib-arm-none-eabi`, `make`를 설치해 빌드합니다. Windows 로컬 빌드와 명령은 같지만, CI는 설치를 자동으로 처리합니다.

PR에서 봐야 할 기준:

| 상황 | 통과해야 하는 항목 |
|---|---|
| 보드 A/B/C/D 중 하나만 수정 | 해당 보드 단위 빌드 + 통합 빌드 |
| `common/` 수정 | 네 보드 단위 빌드 + 통합 빌드 |
| 문서만 수정 | `layout` 통과 |

## 8. 브랜치 전략

```text
main                       검증된 통합
├── feature/can-bsp        공통 CAN BSP
├── feature/engine-sim     보드 A
├── feature/gateway        보드 B
├── feature/uds-server     보드 C
├── feature/body-bcm       보드 D
├── feature/cluster-can    계기판
└── feature/integration    통합 테스트
```

규칙:

1. `main` 직접 push 금지, 반드시 PR 사용.
2. `common/` 수정은 팀장 승인 필수.
3. PR 전 담당 보드 단위 빌드와 루트 통합 빌드 확인.
4. `.ioc`, `Core/`, `Drivers/`, `Middlewares/`를 CubeMX로 재생성했다면 PR 본문에 명시.
5. 자동 생성 파일을 대량 변경했을 때는 어떤 보드에서 재생성했는지 적기.

## 9. 커밋 메시지

```text
[모듈] 간단한 요약
```

예시:

```text
[engine-sim] DBC 기반 RPM 프레임 인코딩 적용
[gateway] UDS 요청 라우팅 ID 변경
[build] 보드별 CMake 독립 빌드 추가
```

태그:

| 태그 | 범위 |
|---|---|
| `[can-bsp]` | `common/can_bsp.*` |
| `[engine-sim]` | `firmware/board_a_engine/src` |
| `[gateway]` | `firmware/board_b_gateway/src` |
| `[uds]` | `firmware/board_c_uds/src` |
| `[body]` | `firmware/board_d_body/src` |
| `[cluster]` | 계기판 관련 코드 |
| `[docs]` | 문서 |
| `[build]` | CMake, workflow, CubeMX 설정 |
| `[fix]` | 버그 수정 |

## 10. 자주 나는 문제

### `arm-none-eabi-gcc is not a full path and was not found`

툴체인이 PATH에 없습니다.

```powershell
where arm-none-eabi-gcc
where arm-none-eabi-g++
```

실패하면 STM32CubeCLT 또는 Arm GNU Toolchain의 `bin` 폴더를 PATH에 추가하세요.

### `make is not recognized`

현재 preset은 `Unix Makefiles` generator를 사용합니다. STM32CubeCLT의 `make` 또는 MSYS2/MinGW의 `make`가 PATH에 있어야 합니다.

### CMake 캐시가 이전 경로를 잡고 있음

빌드 폴더를 삭제하고 다시 configure합니다.

```powershell
Remove-Item -Recurse -Force firmware/board_c_uds/build
cmake --preset Debug --fresh -S firmware/board_c_uds
```

### `common/` 헤더를 못 찾음

보드별 `CMakeLists.txt`에 이미 `../../common` include가 들어가 있습니다. STM32CubeIDE에서만 안 보이면 해당 보드 프로젝트의 include path에 루트 `common` 폴더를 추가하세요.
