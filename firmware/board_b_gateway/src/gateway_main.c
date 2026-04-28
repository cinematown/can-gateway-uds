#include "can_bsp.h"
#include "uart.h"
#include "signal_db.h"
#include <stdio.h>

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2; 

static void Gateway_ForwardToCan2(const CAN_RxMessage_t *rxMsg)
{
    (void)CAN_BSP_SendTo(&hcan2, rxMsg->id, (uint8_t *)rxMsg->data, rxMsg->dlc);
}

void StartDefaultTask(void *argument)
{
    uartInit();
    CAN_BSP_Init();

    uartPrintf(0, "\r\n[Gateway] Central Gateway Started.\r\n");
    
    CAN_RxMessage_t rxMsg;

    for(;;)
    {
        if (CAN_BSP_Read(&rxMsg, osWaitForever))
        {
            if (rxMsg.bus == 1) 
            {
                if (rxMsg.id == CAN_ID_ENGINE_RPM)
                {
                    uint16_t rpm = SignalDb_DecodeMotor1Rpm(rxMsg.data);

                    if (rpm >= 5000) {
                        uint8_t warning_data[8] = {0xFF, 0x01, 0, 0, 0, 0, 0, 0};
                        (void)CAN_BSP_SendTo(&hcan2, CAN_ID_WARNING, warning_data, 8);
                        uartPrintf(0, "[WARN] High RPM: %d\r\n", rpm);
                    }

                    Gateway_ForwardToCan2(&rxMsg);
                    uartPrintf(0, "[Route] RPM CAN1 -> CAN2 | %d rpm\r\n", rpm);
                }
                else if (rxMsg.id == CAN_ID_ENGINE_SPEED ||
                         rxMsg.id == CAN_ID_ENGINE_COOLANT ||
                         rxMsg.id == CAN_ID_ENGINE_KEEPALIVE)
                {
                    Gateway_ForwardToCan2(&rxMsg);
                }
            }
            else if (rxMsg.bus == 2) 
            {
                if (rxMsg.id == CAN_ID_UDS_REQ_BOARD_B) 
                {
                    uartPrintf(0, "[Route] UDS Req CAN2 -> CAN1\r\n");
                    (void)CAN_BSP_SendTo(&hcan1, rxMsg.id, rxMsg.data, rxMsg.dlc);
                }
            }
        }  
    }
}
