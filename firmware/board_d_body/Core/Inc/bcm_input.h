/**
 * @file    bcm_input.h
 * @brief   BCM DIP switch and button input handling.
 */

#ifndef BCM_INPUT_H
#define BCM_INPUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t door_fl;
    uint8_t door_fr;
    uint8_t door_rl;
    uint8_t door_rr;
    uint8_t turn_left_enabled;
    uint8_t turn_right_enabled;
    uint8_t high_beam;
    uint8_t fog_light;
} BcmInput_State_t;

void BCM_Input_Init(void);
void BCM_Input_Poll(void);
void BCM_Input_GetState(BcmInput_State_t *out_state);

#ifdef __cplusplus
}
#endif

#endif /* BCM_INPUT_H */
