#include "uds_service.h"

#include "cli.h"
#include "protocol_ids.h"
#include <stdbool.h>

void UDS_Request_ReadData(uint32_t canId, uint16_t did)
{
    uint8_t txData[8] = {0};

    txData[0] = 0x03;
    txData[1] = 0x22;
    txData[2] = (uint8_t)(did >> 8);
    txData[3] = (uint8_t)(did & 0xFF);

    if (CAN_BSP_Send(canId, txData, 8) != HAL_OK) {
        cliPrintf("[CAN TX Error]\r\n");
    }
}

void UDS_Execute_Diagnostic(uint16_t did, const char* label)
{
    CAN_RxMessage_t rxMsg;
    uint32_t timeout = HAL_GetTick() + 500;
    bool success = false;

    UDS_Request_ReadData(CAN_ID_UDS_REQ_BOARD_B, did);
    cliPrintf("\r\n[TX] Request %s (DID: 0x%04X) via ID: 0x%03X\r\n", label, did, CAN_ID_UDS_REQ_BOARD_B);

    while (HAL_GetTick() < timeout) {
        if (CAN_BSP_GetRxMessage(&rxMsg) == HAL_OK) {
            cliPrintf("[RX] ID=0x%03X Data=", (unsigned int)rxMsg.id);
            for(int i = 0; i < 8; i++) cliPrintf("%02X ", rxMsg.data[i]);
            cliPrintf("\r\n");

            if (rxMsg.id == CAN_ID_UDS_RESP_BOARD_B) {
                uint16_t receivedDid = (rxMsg.data[2] << 8) | rxMsg.data[3];
                
                if (rxMsg.data[1] == 0x62 && receivedDid == did) {
                    cliPrintf("Rx\r\n");
                    cliPrintf("CAN ID : 0x%03X\r\n", (unsigned int)rxMsg.id);
                    cliPrintf("DATA   : ");
                    for(int i = 0; i < 8; i++) {
                        cliPrintf("%02X ", rxMsg.data[i]);
                    }
                    cliPrintf("\r\n");

                    success = true;
                    break; // 원하는 데이터를 찾았으므로 루프 탈출
                }
            }
        }
        osDelay(1); // RTOS 스케줄링을 위해 짧은 지연
    }

    if (!success) {
        cliPrintf(">> [Error] No Valid Response from 0x%03X for DID 0x%04X\r\n", 
                  CAN_ID_UDS_RESP_BOARD_B, did);
    }
}
