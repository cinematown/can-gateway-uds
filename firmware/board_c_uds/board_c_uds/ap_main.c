#include "main.h"
#include "cmsis_os2.h"
#include "uart.h"
#include "cli.h"
#include "can_bsp.h"
#include "uds_client.h"
#include "cli_cmd.h"

/*
 * freertos.c의 __weak UDSMainTask()를 Board C 폴더에서 오버라이딩합니다.
 *
 * Board C 역할:
 * - CAN1로 계기판(Instrument Cluster)에 UDS 요청 송신
 * - 계기판이 보내는 UDS 응답 수신 및 CLI 출력
 *
 * 내부 테스트용 UDS Server / SignalDB 흐름은 제거했습니다.
 */
void UDSMainTask(void *argument)
{
    CAN_RxMessage_t msg;

    (void)argument;

    uartInit();
    cliInit();

    if (CAN_BSP_Init() != HAL_OK)
    {
        cliPrintf("\r\n[ERROR] CAN1 BSP init failed\r\n");
    }

    UDS_Client_Init();
    CLI_CMD_Init();

    cliPrintf("\r\n========================================\r\n");
    cliPrintf(" Board C - Cluster UDS Client Ready\r\n");
    cliPrintf(" Role: CAN1 query to instrument cluster\r\n");
    cliPrintf(" Cluster Req : 0x%03X  (Board C -> Cluster)\r\n", CAN_ID_CLUSTER_UDS_REQ);
    cliPrintf(" Cluster Resp: 0x%03X  (Cluster -> Board C)\r\n", CAN_ID_CLUSTER_UDS_RESP);
    cliPrintf(" Try: read vin  (team-style: resend every 3 sec, any key stop)\r\n");
    cliPrintf("========================================\r\n");
    cliPrintf("CLI > ");

    for (;;)
    {
        cliMain();

        while (CAN_BSP_Read(&msg, 0u))
        {
            /* CAN1 raw log + 계기판 UDS 응답 처리 */
            UDS_Client_OnCanRx(&msg);
        }

        osDelay(1u);
    }
}
