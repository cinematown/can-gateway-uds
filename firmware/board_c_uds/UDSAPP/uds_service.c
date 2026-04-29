#include "cli.h"

void UDS_Execute_Diagnostic(uint16_t did, const char* label) {
    CAN_RxMessage_t rxMsg;

    cliPrintf("\r\n[START] %s 진단 시작 - 3초마다 TX, 아무 키나 누르면 종료\r\n", label);

    for (;;) {
        // 1. TX 전송
        UDS_Request_ReadData(CAN_ID_UDS_REQ_BOARD_B, did);
        cliPrintf("[TX] Request %s (DID: 0x%04X) via ID: 0x%03X\r\n",
                  label, did, CAN_ID_UDS_REQ_BOARD_B);

        // 2. 3초 동안 수신 대기
        uint32_t timeout = HAL_GetTick() + 3000;
        while (HAL_GetTick() < timeout) {

            if (CAN_BSP_GetRxMessage(&rxMsg) == HAL_OK) {
                // 어떤 ID든 일단 전부 출력
                cliPrintf("[RX] ID=0x%03X DLC=%d Data=",
                          (unsigned int)rxMsg.id, rxMsg.dlc);
                for (int i = 0; i < rxMsg.dlc; i++) {
                    cliPrintf("%02X ", rxMsg.data[i]);
                }
                cliPrintf("\r\n");

                // 원하는 ID(0x77E) 수신 시 별도 표시
                if (rxMsg.id == 0x77E) {
                    cliPrintf(">>> [HIT] 0x77E 수신 성공!\r\n");
                    cliPrintf(">>> Data: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                              rxMsg.data[0], rxMsg.data[1], rxMsg.data[2], rxMsg.data[3],
                              rxMsg.data[4], rxMsg.data[5], rxMsg.data[6], rxMsg.data[7]);
                }
            }

            // UART에 뭔가 입력되면 종료
            if (uartAvailable(0) > 0) {
                uint8_t dummy;
                uartReadBlock(0, &dummy, 0);
                cliPrintf("\r\n[STOP] 진단 종료\r\n");
                cliPrintf("CLI> ");
                return;
            }

            osDelay(1);
        }

        // 3초 동안 아무것도 못 받았으면
        cliPrintf("[WAIT] 3초 경과, 재전송...\r\n\r\n");
    }
}