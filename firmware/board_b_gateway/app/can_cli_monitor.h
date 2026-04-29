#ifndef CAN_CLI_MONITOR_H
#define CAN_CLI_MONITOR_H

#include "can_bsp.h"

#include <stdint.h>

void CanCliMonitor_Init(void);
void CanCliMonitor_LogRx(const CAN_RxMessage_t *rx_msg);
void CanCliMonitor_LogTx(uint8_t bus, uint32_t id, const uint8_t *data,
                         uint8_t dlc, HAL_StatusTypeDef status);

#endif
