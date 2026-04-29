#ifndef GATEWAY_ENGINE_BRIDGE_H
#define GATEWAY_ENGINE_BRIDGE_H

#include "can_bsp.h"

void GatewayEngineBridge_OnRx(const CAN_RxMessage_t *rx_msg);
void GatewayEngineBridge_Task10ms(void);

#endif
