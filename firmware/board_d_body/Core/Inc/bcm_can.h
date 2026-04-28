/**
 * @file    bcm_can.h
 * @brief   BCM CAN transmit and receive handling.
 */

#ifndef BCM_CAN_H
#define BCM_CAN_H

#include <stdint.h>
#include "protocol_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t tx_error_count;
} BcmCan_Stats_t;

int BCM_Can_Init(void);
int BCM_Can_SendBodyStatus(const CAN_Msg_t *msg);
void BCM_Can_PollRx(uint32_t timeout_ms);
void BCM_Can_OnRx(const CAN_Msg_t *msg);

uint8_t BCM_Can_IsIgnOn(void);
void BCM_Can_GetStats(BcmCan_Stats_t *out_stats);

#ifdef __cplusplus
}
#endif

#endif /* BCM_CAN_H */
