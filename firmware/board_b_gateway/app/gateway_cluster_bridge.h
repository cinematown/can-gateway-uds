#ifndef GATEWAY_CLUSTER_BRIDGE_H
#define GATEWAY_CLUSTER_BRIDGE_H

#include "can_bsp.h"

void GatewayClusterBridge_OnRx(const CAN_RxMessage_t *rx_msg);
void GatewayClusterBridge_Task10ms(void);

#endif
