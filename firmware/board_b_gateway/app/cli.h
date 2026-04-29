#ifndef __HW_DRIVER_CLI_H__
#define __HW_DRIVER_CLI_H__

#include <stdint.h>  
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "uart.h"

//클린 아키텍처 기반 의존성 역전(DI) 리팩토링
typedef void (*cli_callback_t)(void);
void cliSetCtrlCHandler(cli_callback_t handler);

void cliInit();
void cliRunCommand();
void cliMain();
void cliPrintf(const char* fmt, ...);
void cliParseArgs(char* line_buf);
bool cliAdd(const char* cmd_str, void (*cmd_func)(uint8_t argc, char* argv[]));

#endif //__HW_DRIVER_CLI_H__