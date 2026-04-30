#ifndef GATEWAY_ENGINE_BRIDGE_H
#define GATEWAY_ENGINE_BRIDGE_H

#include "can_bsp.h"

typedef struct {
    uint16_t rpm;
    uint16_t speed_kmh;
    uint8_t coolant_c;
    uint8_t board_a_alive;
    uint8_t ign_on;
    uint8_t active;
    uint32_t age_ms;
} GatewayEngineBridge_State_t;

void GatewayEngineBridge_OnRx(const CAN_RxMessage_t *rx_msg);
void GatewayEngineBridge_Task10ms(void);
void GatewayEngineBridge_GetState(GatewayEngineBridge_State_t *state);

#endif
