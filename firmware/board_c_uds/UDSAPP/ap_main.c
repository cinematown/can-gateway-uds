/* board_c_uds/ap_main.c */
#include "ap_main.h"
#include "uart.h"
#include "cli.h"
#include "can_bsp.h"
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
        /* 4. UDS 응답 처리 */
        // 만약 CLI 입력이 없더라도 CAN 응답을 계속 체크해야 한다면,
        // cliMain 내부의 timeout을 조정하거나 별도의 태스크로 분리하는 게 좋습니다.
        // 현재 구조에서는 한 글자 칠 때마다 한 번씩 체크하게 됩니다.
        UDS_Server_Process(); 
    }
}