/**
 * @file    bcm_signal.c
 * @brief   BCM CAN signal packing.
 */

#include "bcm_signal.h"
#include <string.h>

#define GOLF6_GK1_STA_TUERKONT_BIT      4U
#define GOLF6_GK1_SLEEP_ACKN_BIT        7U
#define GOLF6_GK1_FA_TUERKONT_BIT       16U
#define GOLF6_GK1_RUECKFAHR_BIT         28U
#define GOLF6_GK1_BLINKER_LI_BIT        34U
#define GOLF6_GK1_BLINKER_RE_BIT        35U
#define GOLF6_GK1_LS1_FERNLICHT_BIT     37U
#define GOLF6_GK1_ABBLENDLICHT_BIT      48U
#define GOLF6_GK1_FERNLICHT_BIT         49U
#define GOLF6_GK1_WARNBLK_STATUS_BIT    55U
#define GOLF6_GK1_NEBEL_EIN_BIT         58U

static void set_bit(uint8_t *data, uint8_t bit, uint8_t on)
{
    uint8_t mask = (uint8_t)(1U << (bit % 8U));

    if (on != 0U) {
        data[bit / 8U] |= mask;
    } else {
        data[bit / 8U] &= (uint8_t)~mask;
    }
}

void BCM_Signal_BuildBodyStatus(const BcmSignal_BodyStatus_t *status, CAN_Msg_t *out_msg)
{
    uint8_t any_door_open;

    if (status == NULL || out_msg == NULL) {
        return;
    }

    memset(out_msg, 0, sizeof(*out_msg));
    out_msg->id = BCM_GOLF6_CAN_ID_MGATE_KOMF_1;
    out_msg->dlc = BCM_GOLF6_MGATE_KOMF_1_DLC;

    any_door_open = status->input.door_fl ||
                    status->input.door_fr ||
                    status->input.door_rl ||
                    status->input.door_rr;

    set_bit(out_msg->data, GOLF6_GK1_STA_TUERKONT_BIT, any_door_open);
    set_bit(out_msg->data, GOLF6_GK1_FA_TUERKONT_BIT, any_door_open);
    set_bit(out_msg->data, GOLF6_GK1_BLINKER_LI_BIT,
            status->input.turn_left_enabled && status->left_blink_on);
    set_bit(out_msg->data, GOLF6_GK1_BLINKER_RE_BIT,
            status->input.turn_right_enabled && status->right_blink_on);
    set_bit(out_msg->data, GOLF6_GK1_LS1_FERNLICHT_BIT, status->input.high_beam);
    set_bit(out_msg->data, GOLF6_GK1_FERNLICHT_BIT, status->input.high_beam);
    set_bit(out_msg->data, GOLF6_GK1_ABBLENDLICHT_BIT, status->input.high_beam);
    set_bit(out_msg->data, GOLF6_GK1_NEBEL_EIN_BIT, status->input.fog_light);

    set_bit(out_msg->data, GOLF6_GK1_SLEEP_ACKN_BIT, 1U);
    set_bit(out_msg->data, GOLF6_GK1_RUECKFAHR_BIT, 1U);
    set_bit(out_msg->data, GOLF6_GK1_WARNBLK_STATUS_BIT, 1U);
}
