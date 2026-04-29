#include "cli.h"

#define CLI_LINE_BUF_MAX 128
#define CLI_CMD_LIST_MAX 32
#define CLI_CMD_ARG_MAX 16


typedef struct _cli_cmd_t {
    char cmd_str[16];
    void (*cmd_func)(uint8_t argc, char** argv);

}cli_cmd_t;

static cli_cmd_t cli_cmd_list[CLI_CMD_LIST_MAX];
static uint8_t cli_cmd_count = 0;

static uint8_t cli_argc;
static char* cli_argv[CLI_CMD_ARG_MAX];
static char cli_line_buf[CLI_LINE_BUF_MAX];
static uint16_t cli_line_idx = 0;

#define CLI_HIST_MAX 10
static char cli_history_buf[CLI_HIST_MAX][CLI_LINE_BUF_MAX];

static uint8_t cli_hist_count = 0;
static uint8_t cli_hist_write = 0;
static uint8_t cli_hist_depth = 0;

typedef enum {
    CLI_ST_NORMAL = 0,
    CLI_ST_ESC_RCVD,  //rcvd = received
    CLI_ST_BRACKET_RCVD
} cli_input_state_t;

static cli_input_state_t input_st = CLI_ST_NORMAL;
static cli_callback_t ctrl_c_handler = NULL;

static void handleEnterkey(void) {
    cliPrintf("\r\n"); // 줄바꿈
    cli_line_buf[cli_line_idx] = '\0'; // 문자열 마무리

    // [수정] 아무것도 입력하지 않은 빈 줄은 무시
    if (cli_line_idx > 0) {
        // [히스토리 저장]
        strncpy(cli_history_buf[cli_hist_write], cli_line_buf, CLI_LINE_BUF_MAX - 1);
        cli_hist_write = (cli_hist_write + 1) % CLI_HIST_MAX;
        cli_hist_depth = 0;

        if (cli_hist_count < CLI_HIST_MAX)
            cli_hist_count++;

        // 명령어 분석 및 실행
        cliParseArgs(cli_line_buf);
        cliRunCommand();
    }

    // 다음 입력을 위한 초기화
    cliPrintf("CLI > ");
    cli_line_idx = 0;
}

static void handleBackspace(void) {
    if (cli_line_idx > 0) {
        cli_line_idx--;      // 배열 인덱스 뒤로 한 칸
        cliPrintf("\b \b");  // 커서 뒤로 -> 공백으로 지움 -> 커서 다시 뒤로
    }
}

static void handleCharInsert(uint8_t rx_data)
{
    if (cli_line_idx >= CLI_LINE_BUF_MAX - 1)
    {
        return;
    }

    cliPrintf("%c", rx_data);
    cli_line_buf[cli_line_idx++] = rx_data;
}

static void handleArrowKeys(uint8_t rx_data) {
    if (rx_data == 'A') {
        // 저장된 명령어 개수 한도 내에서만 더 과거로 이동 가능
        if (cli_hist_depth < cli_hist_count) {
            cli_hist_depth++; // 한 단계 더 과거로!

            // 1. 현재 화면에 쳐져 있던 글자를 싹 지웁니다.
            for (uint16_t i = 0; i < cli_line_idx; i++) {
                cliPrintf("\b \b");
            }

            // 2. 현재 쓰는 위치에서 깊이만큼 빼서 '읽을 위치(idx)'를 계산 (원형 버퍼 수학)
            int idx = (cli_hist_write + CLI_HIST_MAX - cli_hist_depth) % CLI_HIST_MAX;

            // 3. 기록장에서 현재 버퍼로 복사
            strncpy(cli_line_buf, cli_history_buf[idx], CLI_LINE_BUF_MAX - 1);
            cli_line_idx = strlen(cli_line_buf); // 인덱스 길이 맞춰주기

            // 4. 불러온 과거 명령어를 화면에 출력
            cliPrintf("%s", cli_line_buf);
        }
    }
    // --- [아래쪽 방향키 ('B')] 최근 명령어로 돌아오기 ---
    else if (rx_data == 'B') {
        if (cli_hist_depth > 0) {
            cli_hist_depth--; // 과거에서 다시 현재 쪽으로 얕아짐

            // 1. 현재 화면 지우기
            for (uint16_t i = 0; i < cli_line_idx; i++) {
                cliPrintf("\b \b");
            }

            // 2. 깊이가 0이 되었다면 (가장 현재) -> 아예 빈 줄로 놔둠
            if (cli_hist_depth == 0) {
                cli_line_buf[0] = '\0';
                cli_line_idx = 0;
            }
            // 3. 아직 과거라면 -> 해당 위치의 명령어를 불러옴
            else {
                int idx = (cli_hist_write + CLI_HIST_MAX - cli_hist_depth) % CLI_HIST_MAX;
                strncpy(cli_line_buf, cli_history_buf[idx], CLI_LINE_BUF_MAX - 1);
                cliPrintf("%s", cli_line_buf);
                cli_line_idx = strlen(cli_line_buf);
            }
        }
    }
    input_st = CLI_ST_NORMAL;
}

static void processAnsiEscape(uint8_t rx_data) {
    if (input_st == CLI_ST_ESC_RCVD) {
        // 표준 방향키는 항상 'ESC' + '[' + 'A/B/C/D' 조합으로 들어옵니다.
        if (rx_data == '[') input_st = CLI_ST_BRACKET_RCVD; // '['가 맞으면 상태 2로 진행
        else                input_st = CLI_ST_NORMAL; // 아니면 방향키가 아니니 취소
    }
    else if (input_st == CLI_ST_BRACKET_RCVD) {
        handleArrowKeys(rx_data);
    }
}

void cliMain(void)
{
    uint8_t rx_data;

    if (uartReadBlock(0, &rx_data, 0) == true)
    {
        if (input_st != CLI_ST_NORMAL)
        {
            processAnsiEscape(rx_data);
            return;
        }

        switch (rx_data)
        {
        case 0x03:
            if (ctrl_c_handler != NULL)
                ctrl_c_handler();

            cliPrintf("^C \r\nCLI > ");
            cli_line_idx = 0;
            break;

        case 0x1B:
            input_st = CLI_ST_ESC_RCVD;
            break;

        case '\r':
            handleEnterkey();
            break;

        case '\n':
            // 터미널에서 \r\n이 올 때 \n을 무시하여 이중 실행 방지
            break;

        case '\b':
        case 127:
            handleBackspace();
            break;

        default:
            if (32 <= rx_data && rx_data <= 126)
                handleCharInsert(rx_data);
            break;
        }
    }
}

static void cliHelp(uint8_t argc, char* argv[]) {
    cliPrintf("-----------CLI Commands-----------\r\n");
    for (uint8_t i = 0; i < cli_cmd_count; i++) {
        cliPrintf("%s \r\n", cli_cmd_list[i].cmd_str);
    }
    cliPrintf("----------------------------------\r\n");
}

static void cliClear(uint8_t argc, char* argv[]) {
    cliPrintf("\x1B[2J\x1B[H");
}

void cliInit() {
    ctrl_c_handler = NULL;
    cli_cmd_count = 0;
    cli_line_idx = 0;

    cliAdd("help", cliHelp);
    cliAdd("cls", cliClear);
}

void cliSetCtrlCHandler(cli_callback_t handler) {
    ctrl_c_handler = handler;
}

void cliPrintf(const char* fmt, ...) {
    char buf[128];
    va_list args;

    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args); // uint32_t -> int 로 변경 (음수 에러 확인용)
    va_end(args);

    if (len < 0) return; // 변환 실패 방어

    // [수정] vsnprintf가 버퍼보다 큰 길이를 반환하더라도 버퍼 크기만큼만 전송하도록 제한
    if (len >= sizeof(buf)) {
        len = sizeof(buf) - 1;
    }

    uartWrite(0, (uint8_t*)buf, len);
}

void cliParseArgs(char* line_buf) {
    char* tok;
    cli_argc = 0;
    tok = strtok(line_buf, " ");
    while (tok != NULL && cli_argc < CLI_CMD_ARG_MAX) {
        cli_argv[cli_argc++] = tok;
        tok = strtok(NULL, " ");
    }
}

void cliRunCommand() {
    if (cli_argc == 0) return;

    bool is_found = false;
    for (uint8_t i = 0; i < cli_cmd_count; i++) {
        if (strcmp(cli_argv[0], cli_cmd_list[i].cmd_str) == 0) {
            is_found = true;
            cli_cmd_list[i].cmd_func(cli_argc, cli_argv);
            break;
        }
    }
    if (is_found == false) {
        cliPrintf("Command Not Found \r\n");
    }
}

bool cliAdd(const char* cmd_str, void (*cmd_func)(uint8_t argc, char* argv[]))
{
    if (cli_cmd_count >= CLI_CMD_LIST_MAX)
        return false;

    strncpy(cli_cmd_list[cli_cmd_count].cmd_str,
        cmd_str,
        sizeof(cli_cmd_list[cli_cmd_count].cmd_str) - 1);

    cli_cmd_list[cli_cmd_count].cmd_str[sizeof(cli_cmd_list[cli_cmd_count].cmd_str) - 1] = '\0';

    cli_cmd_list[cli_cmd_count].cmd_func = cmd_func;
    cli_cmd_count++;

    return true;
}

