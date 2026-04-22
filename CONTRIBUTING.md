# Contributing Guide

## 1. 시작하기

### 첫 클론 후
```bash
git clone https://github.com/<OWNER>/can-gateway-uds.git
cd can-gateway-uds

# 자기 담당 브랜치로 이동
git checkout feature/<your-module>

# STM32CubeIDE에서 자기 담당 보드 프로젝트 Import
# File → Import → Existing Projects → board_a_engine/ (예)
```

### `common/` 사용법
각 보드 프로젝트에서 `common/` 폴더를 Include Path 에 추가해야 합니다.

**STM32CubeIDE 설정**:
1. 프로젝트 우클릭 → Properties
2. C/C++ General → Paths and Symbols → Includes → GNU C
3. Add → Workspace → `../common/can_bsp` 와 `../common/can_db` 추가
4. C/C++ General → Paths and Symbols → Source Location
5. Link Folder → `../common/can_bsp` 링크 (빌드 대상에 포함)

## 2. 브랜치 전략

```
main                       ← 검증된 통합 (팀장만 머지)
├── feature/can-bsp        ← ① 성재/민진
├── feature/engine-sim     ← ② 윤지
├── feature/gateway        ← ③ 지윤
├── feature/uds-server     ← ④ 은빈
├── feature/cluster-can    ← ⑤ 미정
└── feature/integration    ← ⑥ 한결 (통합 테스트)
```

### 규칙
1. **`main` 직접 push 절대 금지** → 반드시 PR
2. **`common/` 수정은 팀장 승인 필수** (인터페이스 변경 = 전원 영향)
3. 자기 브랜치는 자유롭게 커밋 가능
4. PR 올리기 전 **자기 브랜치 로컬에서 빌드 성공 확인**
5. 1일 1회 이상 push 권장 (백업 + 진행 상황 공유)

## 3. 커밋 메시지 규칙

```
[모듈] 간단한 요약 (50자 이내)

(선택) 상세 설명
- 뭘 했는지
- 왜 했는지
- 주의사항
```

### 예시
```
[engine-sim] ADC2 채널로 브레이크 페달 입력 추가

- PA1 포텐셔미터 연결
- 브레이크 > 50% 시 속도 절반 감속
- TODO: 감속 곡선 튜닝 필요
```

### 태그 목록
- `[can-bsp]` CAN 드라이버
- `[engine-sim]` 엔진 시뮬레이터
- `[gateway]` 게이트웨이
- `[uds]` UDS 서버
- `[cluster]` 계기판
- `[main]` main.c / Task 구조
- `[docs]` 문서
- `[build]` 빌드/CubeMX 설정
- `[fix]` 버그 수정

## 4. Pull Request 규칙

### PR 제목
```
[모듈] 변경사항 요약
```

### PR 본문
`.github/PULL_REQUEST_TEMPLATE.md` 자동 로딩됨. 체크리스트 모두 채울 것.

### 리뷰어
- 일반 PR: 팀장(⑥) 1명
- `common/` 수정 PR: 팀장(⑥) + 영향받는 모듈 담당자
- 통합 PR: 전원

### 머지 방식
- **Squash and merge** 기본 (히스토리 깔끔)
- 대규모 feature는 **Rebase and merge** (커밋 보존)

## 5. 일정 체크포인트

### 1주차
- **Day 2**: 팀장 헤더 파일 배포 완료
- **Day 4**: ① CAN 드라이버 실 CAN 양방향 통신 성공 (**크리티컬 패스**)
- **Day 7**: 각 모듈 단위 테스트 완료 (loopback/stub 기반)

### 2주차
- **Day 8~11**: 쌍별 통합 (①+②, ①+③, ③+⑤, ①+④)
- **Day 12**: 엔드투엔드 통합 테스트
- **Day 13~14**: 버그 수정, 문서, 영상

## 6. 충돌 대응

### "내 브랜치에서 빌드 안 돼요"
1. `git status` 확인 → CubeMX 재생성 흔적 있는지 (`Drivers/`, `Core/` 변경)
2. 최신 `main` pull 후 rebase
3. 해결 안 되면 팀장 호출 (`.ioc` 수정은 팀장만)

### "머지 충돌 났어요"
1. **절대 함부로 resolve 하지 말 것**
2. 팀장에게 DM → 원인 파악 후 같이 해결
3. `common/` 충돌은 특히 주의

### "다른 사람 코드 필요해요"
```bash
git fetch origin
git merge origin/main       # 또는 rebase
```
아직 머지 안 된 다른 feature 브랜치 기능이 필요하면 팀장에게 조기 통합 요청.

## 7. 일일 스탠드업 (매일 10분)

각자 답할 것:
1. 어제 뭐 했나?
2. 오늘 뭐 할 건가?
3. 막힌 것 / 남에게 의존하는 것?

카톡/Slack 비동기로 아침에 공유 권장.
