/**
 * @file    bcm_signal.h
 * @brief   BCM CAN signal packing.
 */

#ifndef BCM_SIGNAL_H
#define BCM_SIGNAL_H

#include <stdint.h>
#include "bcm_input.h"
#include "protocol_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BCM_GOLF6_CAN_ID_MGATE_KOMF_1 0x390U
#define BCM_GOLF6_MGATE_KOMF_1_DLC    8U

typedef struct {
    BcmInput_State_t input;
    uint8_t left_blink_on;
    uint8_t right_blink_on;
} BcmSignal_BodyStatus_t;

void BCM_Signal_BuildBodyStatus(const BcmSignal_BodyStatus_t *status, CAN_Msg_t *out_msg);

#ifdef __cplusplus
}
#endif

#endif /* BCM_SIGNAL_H */
