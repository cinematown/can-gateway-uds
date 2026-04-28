/**
 * @file    bcm_signal.c
 * @brief   BCM CAN signal packing.
 */

#include "bcm_signal.h"
#include <string.h>

void BCM_Signal_BuildBodyStatus(const BcmSignal_BodyStatus_t *status, CAN_Msg_t *out_msg)
{
    if (status == NULL || out_msg == NULL) {
        return;
    }

    memset(out_msg, 0, sizeof(*out_msg));
    out_msg->id = CAN_ID_BODY_STATUS;
    out_msg->dlc = CAN_DLC_BODY_STATUS;

    VW470_SET_TURN_LEFT(out_msg->data,
                        status->input.turn_left_enabled && status->left_blink_on);
    VW470_SET_TURN_RIGHT(out_msg->data,
                         status->input.turn_right_enabled && status->right_blink_on);
    VW470_SET_DOOR_FL(out_msg->data, status->input.door_fl);
    VW470_SET_DOOR_FR(out_msg->data, status->input.door_fr);
    VW470_SET_DOOR_RL(out_msg->data, status->input.door_rl);
    VW470_SET_DOOR_RR(out_msg->data, status->input.door_rr);
    VW470_SET_HIGH_BEAM(out_msg->data, status->input.high_beam);
    VW470_SET_FOG_LIGHT(out_msg->data, status->input.fog_light);
}
