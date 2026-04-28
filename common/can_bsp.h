#ifndef CAN_BSP_H
#define CAN_BSP_H

#include "main.h"
#include "cmsis_os2.h"
#include "protocol_ids.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t  bus;
    uint32_t id;
    uint8_t  dlc;
    uint8_t  data[8];
    CAN_RxHeaderTypeDef header;
} CAN_RxMessage_t;

extern osMessageQueueId_t can_rx_q;

extern volatile uint32_t can1TxCount;
extern volatile uint32_t can1RxCount;
extern volatile uint32_t can2TxCount;
extern volatile uint32_t can2RxCount;
extern volatile uint32_t canSendEnterCount;
extern volatile uint32_t canTxBusyCount;
extern volatile uint32_t canTxErrorCount;

HAL_StatusTypeDef CAN_BSP_Init(void);
HAL_StatusTypeDef CAN_BSP_Send(uint32_t id, uint8_t *data, uint8_t len);
HAL_StatusTypeDef CAN_BSP_SendTo(CAN_HandleTypeDef *hcan, uint32_t id, uint8_t *data, uint8_t len);
bool CAN_BSP_Read(CAN_RxMessage_t *p_msg, uint32_t timeout);

void CAN_BSP_ConfigFilter_Open(void);
void CAN_BSP_ConfigFilter(uint32_t id);
void CAN_BSP_ConfigFilter_UDS_Response(void);
HAL_StatusTypeDef CAN_BSP_GetRxMessage(CAN_RxMessage_t *p_msg);

#endif
