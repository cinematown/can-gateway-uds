#include "cli.h"
#include <string.h>
#include <stdlib.h>

#define CLI_LINE_BUF_MAX 64
#define CLI_CMD_LIST_MAX 16
#define CLI_CMD_ARG_MAX  4
#define CLI_HIST_MAX     5

typedef struct {
    char cmd_str[16];
    void (*cmd_func)(uint8_t argc, char **argv);
} cli_cmd_t;

static cli_cmd_t cli_cmd_list[CLI_CMD_LIST_MAX];
static uint8_t   cli_cmd_count = 0;
static uint8_t   cli_argc = 0;
static char     *cli_argv[CLI_CMD_ARG_MAX];
static char      cli_line_buf[CLI_LINE_BUF_MAX];
static uint16_t  cli_line_idx = 0;

// 히스토리 관리 변수
static char      cli_history_buf[CLI_HIST_MAX][CLI_LINE_BUF_MAX];
static uint8_t   cli_hist_write = 0;
static uint8_t   cli_hist_depth = 0;
static uint8_t   cli_hist_count = 0;

typedef enum { CLI_STATE_NORMAL = 0, CLI_STATE_ESC, CLI_STATE_BRACKET } cli_state_t;
static cli_state_t input_state = CLI_STATE_NORMAL;

/* --- 내부 처리 함수 --- */

static void handleEnterKey(void) {
    cliPrintf("\r\n");
    if (cli_line_idx > 0) {
        cli_line_buf[cli_line_idx] = '\0';
        // 히스토리 저장
        strncpy(cli_history_buf[cli_hist_write], cli_line_buf, CLI_LINE_BUF_MAX);
        cli_hist_write = (cli_hist_write + 1) % CLI_HIST_MAX;
        if (cli_hist_count < CLI_HIST_MAX) cli_hist_count++;
        cli_hist_depth = 0;

        cliParseArgs(cli_line_buf);
        cliRunCommand();
    }
    cliPrintf("CLI> ");
    cli_line_idx = 0;
}

static void handleBackspace(void) {
    if (cli_line_idx > 0) {
        cli_line_idx--;
        cliPrintf("\b \b");
    }
}

static void handleArrowKeys(uint8_t data) {
    if (data == 'A' || data == 'B') { // 위/아래 화살표
        // 이전 줄 지우기
        for (int i = 0; i < cli_line_idx; i++) cliPrintf("\b \b");

        if (data == 'A' && cli_hist_depth < cli_hist_count) cli_hist_depth++;
        else if (data == 'B' && cli_hist_depth > 0) cli_hist_depth--;

        if (cli_hist_depth > 0) {
            int idx = (cli_hist_write + CLI_HIST_MAX - cli_hist_depth) % CLI_HIST_MAX;
            strncpy(cli_line_buf, cli_history_buf[idx], CLI_LINE_BUF_MAX);
        } else {
            cli_line_buf[0] = '\0';
        }
        cli_line_idx = strlen(cli_line_buf);
        cliPrintf("%s", cli_line_buf);
    }
}

/* --- 외부 공개 함수 --- */
/* UDSAPP/cli.c */

/* UDSAPP/cli.c */
#include "can_bsp.h" // UDS_Request_ReadData 호출을 위해 필요

/* cli.c */
#include "can_bsp.h"

void cliRead(uint8_t argc, char **argv) {
    if (argc < 2) {
        cliPrintf("Usage: read [vin|rpm|speed|temp]\r\n");
        return;
    }

    // 문자열에 따라 DID 매핑 (ID는 uds_service가 알아서 처리)
    if      (strcmp(argv[1], "vin") == 0)   UDS_Execute_Diagnostic(UDS_DID_VIN, "VIN");
    else if (strcmp(argv[1], "rpm") == 0)   UDS_Execute_Diagnostic(UDS_DID_RPM, "RPM");
    else if (strcmp(argv[1], "speed") == 0) UDS_Execute_Diagnostic(UDS_DID_SPEED, "SPEED");
    else if (strcmp(argv[1], "temp") == 0)  UDS_Execute_Diagnostic(UDS_DID_TEMP, "TEMP");
    else                                    cliPrintf("Unknown item: %s\r\n", argv[1]);
}

void cliInit(void) {
    cli_cmd_count = 0;
    cliAdd("help", cliHelp);
    cliAdd("cls",  cliClear);
    cliAdd("read", cliRead);
}

void cliMain(void) {
    uint8_t rx_data;
    if (uartReadBlock(0, &rx_data, osWaitForever)) {
        if (input_state == CLI_STATE_ESC) {
            if (rx_data == '[') input_state = CLI_STATE_BRACKET;
            else input_state = CLI_STATE_NORMAL;
        } else if (input_state == CLI_STATE_BRACKET) {
            handleArrowKeys(rx_data);
            input_state = CLI_STATE_NORMAL;
        } else {
            switch (rx_data) {
                case 0x1B: input_state = CLI_STATE_ESC; break;
                case '\r':
                case '\n': handleEnterKey(); break;
                case 0x08:
                case 127:  handleBackspace(); break;
                default:
                    if (rx_data >= 32 && rx_data <= 126 && cli_line_idx < CLI_LINE_BUF_MAX - 1) {
                        cliPrintf("%c", rx_data);
                        cli_line_buf[cli_line_idx++] = rx_data;
                    }
                    break;
            }
        }
    }
}

// 반환 타입을 void에서 bool로 변경합니다.
bool cliAdd(const char *cmd_str, void (*cmd_func)(uint8_t argc, char **argv)) {
    if (cli_cmd_count < CLI_CMD_LIST_MAX) {
        // 안전한 복사를 위해 strncpy 사용
        strncpy(cli_cmd_list[cli_cmd_count].cmd_str, cmd_str, 15);
        cli_cmd_list[cli_cmd_count].cmd_str[15] = '\0'; // 널 문자로 마무리
        
        cli_cmd_list[cli_cmd_count].cmd_func = cmd_func;
        cli_cmd_count++;
        return true;  // 성공 시 true 반환
    }
    return false; // 리스트가 꽉 찼을 때 false 반환
}

void cliParseArgs(char *line_buf) {
    char *tok = strtok(line_buf, " ");
    cli_argc = 0;
    while (tok != NULL && cli_argc < CLI_CMD_ARG_MAX) {
        cli_argv[cli_argc++] = tok;
        tok = strtok(NULL, " ");
    }
}

void cliRunCommand(void) {
    if (cli_argc == 0) return;
    for (int i = 0; i < cli_cmd_count; i++) {
        if (strcmp(cli_argv[0], cli_cmd_list[i].cmd_str) == 0) {
            cli_cmd_list[i].cmd_func(cli_argc, cli_argv);
            return;
        }
    }
    cliPrintf("Unknown Command: %s\r\n", cli_argv[0]);
}

void cliHelp(uint8_t argc, char *argv[]) {
    cliPrintf("\r\n---- Available Commands ----\r\n");
    for (int i = 0; i < cli_cmd_count; i++) cliPrintf("  %s\r\n", cli_cmd_list[i].cmd_str);
}

void cliClear(uint8_t argc, char *argv[]) {
    cliPrintf("\x1B[2J\x1B[H");
}

void cliPrintf(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    uartWrite(0, (uint8_t *)buf, len);
}