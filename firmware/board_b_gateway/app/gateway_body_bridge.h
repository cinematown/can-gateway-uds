#ifndef GATEWAY_BODY_BRIDGE_H
#define GATEWAY_BODY_BRIDGE_H

#include "can_bsp.h"

void GatewayBodyBridge_OnRx(const CAN_RxMessage_t *rx_msg);
void GatewayBodyBridge_Task10ms(void);

#endif
