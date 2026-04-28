/* board_c_uds/ap_main.c */
#include "ap_main.h"
#include "uart.h"
#include "cli.h"
#include "can_bsp.h"
#include "uds_cli_cmd.h"
#include "uds_server.h"

/**
 * @brief FreeRTOS의 기본 태스크 오버라이딩
 */
void StartDefaultTask(void *argument)
{
    /* 1. 하드웨어 및 RTOS 자원 초기화 */
    // uartInit 내부에 Queue와 Mutex 생성이 들어있으므로 반드시 먼저 호출!
    uartInit();        
    
    // CLI 명령어(help, cls 등) 등록
    cliInit();         
    UDS_CLI_Init();
    
    // CAN 및 UDS 관련 초기화
    CAN_BSP_Init();    
    UDS_Server_Init(); 

    /* 2. 시작 환영 문구 출력 */
    // 이제 cliPrintf나 uartPrintf를 통해 Mutex 보호를 받으며 안전하게 출력됩니다.
    cliPrintf("\x1B[2J\x1B[H"); // 화면 지우기 (ANSI Escape)
    cliPrintf("==================================\r\n");
    cliPrintf("    [Board C] UDS Diagnostic Tool  \r\n");
    cliPrintf("    UART3 / 115200bps / 180MHz     \r\n");
    cliPrintf("==================================\r\n");
    cliPrintf("CLI> ");

    for(;;)
    {
        cliMain();
        UDS_Server_Process(); 
        osDelay(1);
    }
}
