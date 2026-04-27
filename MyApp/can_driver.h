#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include "main.h"
#include "engine_can.h"
#include <stdint.h>

HAL_StatusTypeDef CAN_Driver_Init(void);
HAL_StatusTypeDef CAN1_Send(uint32_t id, uint8_t *data, uint8_t len);

extern volatile uint32_t can1TxCount;
extern volatile uint32_t canSendEnterCount;
extern volatile uint32_t canTxBusyCount;
extern volatile uint32_t canTxErrorCount;

#endif