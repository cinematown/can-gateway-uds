#ifndef CAN_RX_DEBUG_H
#define CAN_RX_DEBUG_H

#include "main.h"
#include <stdint.h>
#include "protocol_ids.h"

HAL_StatusTypeDef CAN_RxDebug_Init(void);
void CAN_RxDebug_Task(void *argument);

extern volatile uint32_t canRxCount;

extern volatile uint16_t rx_rpm;
extern volatile uint16_t rx_speed;
extern volatile uint8_t rx_coolant;
extern volatile uint8_t rx_throttle;
extern volatile uint8_t rx_brake;

#endif
