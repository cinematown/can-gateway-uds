#ifndef CLI_H
#define CLI_H

#include "hw_def.h"
#include "uart.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "protocol_ids.h"
#include "uds_service.h"

/* --- 타입 정의 --- */

// 특정 이벤트(예: Ctrl+C) 발생 시 실행될 콜백 함수 타입
typedef void (*cli_callback_t)(void);

/* --- 외부 공개 함수 선언 --- */

/**
 * @brief CLI 엔진 초기화 (명령어 리스트 비우기 및 기본 명령어 등록)
 */
void cliInit(void);

/**
 * @brief CLI 메인 루프 (RTOS 태스크 내에서 반복 호출되어 입력을 처리)
 */

 void cliRead(uint8_t argc, char **argv);
void cliMain(void);

/**
 * @brief 새로운 CLI 명령어를 등록
 * @param cmd_str 명령어 문자열 (예: "read")
 * @param cmd_func 실행될 함수 포인터
 * @return 성공 여부
 */
bool cliAdd(const char *cmd_str, void (*cmd_func)(uint8_t argc, char **argv));

/**
 * @brief printf 스타일의 CLI 전용 출력 함수
 */
void cliPrintf(const char *fmt, ...);

/**
 * @brief 입력된 문자열을 인자(Argument) 단위로 분리
 */
void cliParseArgs(char *line_buf);

/**
 * @brief 분리된 인자를 바탕으로 등록된 명령어를 찾아 실행
 */
void cliRunCommand(void);

/**
 * @brief Ctrl+C 같은 제어 신호 처리용 핸들러 등록
 */
void cliSetCtrlHandler(cli_callback_t handler);

/* --- 기본 명령어 함수 (cli.c 내부에 구현됨) --- */
void cliHelp(uint8_t argc, char *argv[]);
void cliClear(uint8_t argc, char *argv[]);

#endif /* CLI_H */