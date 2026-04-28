/**
 * @file    bcm_input.c
 * @brief   BCM DIP switch and button input handling.
 */

#include "bcm_input.h"
#include "main.h"
#include <string.h>

/* Inputs use pull-up wiring: active switch/button shorts the pin to GND. */
#ifndef BCM_INPUT_ACTIVE_STATE
#define BCM_INPUT_ACTIVE_STATE GPIO_PIN_RESET
#endif

#if !defined(BCM_DOOR_FL_GPIO_Port) && (defined(STM32F103xB) || defined(STM32F103RB))
#define BCM_DOOR_FL_GPIO_Port      GPIOB
#define BCM_DOOR_FL_Pin            GPIO_PIN_0
#define BCM_DOOR_FR_GPIO_Port      GPIOB
#define BCM_DOOR_FR_Pin            GPIO_PIN_1
#define BCM_DOOR_RL_GPIO_Port      GPIOB
#define BCM_DOOR_RL_Pin            GPIO_PIN_2
#define BCM_DOOR_RR_GPIO_Port      GPIOB
#define BCM_DOOR_RR_Pin            GPIO_PIN_10
#define BCM_TURN_LEFT_GPIO_Port    GPIOB
#define BCM_TURN_LEFT_Pin          GPIO_PIN_11
#define BCM_TURN_RIGHT_GPIO_Port   GPIOB
#define BCM_TURN_RIGHT_Pin         GPIO_PIN_12
#define BCM_HIGH_BEAM_GPIO_Port    GPIOB
#define BCM_HIGH_BEAM_Pin          GPIO_PIN_13
#define BCM_FOG_LAMP_GPIO_Port     GPIOB
#define BCM_FOG_LAMP_Pin           GPIO_PIN_14
#endif

#ifndef BCM_DOOR_FL_GPIO_Port
#define BCM_DOOR_FL_GPIO_Port      GPIOE
#define BCM_DOOR_FL_Pin            GPIO_PIN_2
#endif
#ifndef BCM_DOOR_FR_GPIO_Port
#define BCM_DOOR_FR_GPIO_Port      GPIOE
#define BCM_DOOR_FR_Pin            GPIO_PIN_3
#endif
#ifndef BCM_DOOR_RL_GPIO_Port
#define BCM_DOOR_RL_GPIO_Port      GPIOE
#define BCM_DOOR_RL_Pin            GPIO_PIN_4
#endif
#ifndef BCM_DOOR_RR_GPIO_Port
#define BCM_DOOR_RR_GPIO_Port      GPIOE
#define BCM_DOOR_RR_Pin            GPIO_PIN_5
#endif
#ifndef BCM_TURN_LEFT_GPIO_Port
#define BCM_TURN_LEFT_GPIO_Port    GPIOE
#define BCM_TURN_LEFT_Pin          GPIO_PIN_6
#endif
#ifndef BCM_TURN_RIGHT_GPIO_Port
#define BCM_TURN_RIGHT_GPIO_Port   GPIOF
#define BCM_TURN_RIGHT_Pin         GPIO_PIN_6
#endif
#ifndef BCM_HIGH_BEAM_GPIO_Port
#define BCM_HIGH_BEAM_GPIO_Port    GPIOF
#define BCM_HIGH_BEAM_Pin          GPIO_PIN_7
#endif
#ifndef BCM_FOG_LAMP_GPIO_Port
#define BCM_FOG_LAMP_GPIO_Port     GPIOF
#define BCM_FOG_LAMP_Pin           GPIO_PIN_8
#endif

#define BCM_BUTTON_DEBOUNCE_SAMPLES 2U

typedef struct {
    uint8_t raw_active;
    uint8_t stable_active;
    uint8_t same_count;
} BcmButton_Debounce_t;

static volatile BcmInput_State_t s_state;
static BcmButton_Debounce_t s_left_button;
static BcmButton_Debounce_t s_right_button;

static uint8_t read_input(GPIO_TypeDef *port, uint16_t pin)
{
    return HAL_GPIO_ReadPin(port, pin) == BCM_INPUT_ACTIVE_STATE ? 1U : 0U;
}

static uint8_t debounce_pressed(BcmButton_Debounce_t *button, uint8_t raw_active)
{
    uint8_t pressed = 0U;

    if (raw_active == button->raw_active) {
        if (button->same_count < BCM_BUTTON_DEBOUNCE_SAMPLES) {
            button->same_count++;
        }
    } else {
        button->raw_active = raw_active;
        button->same_count = 0U;
    }

    if (button->same_count >= BCM_BUTTON_DEBOUNCE_SAMPLES &&
        raw_active != button->stable_active) {
        button->stable_active = raw_active;
        if (button->stable_active) {
            pressed = 1U;
        }
    }

    return pressed;
}

void BCM_Input_Init(void)
{
    GPIO_InitTypeDef gpio;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
#ifdef __HAL_RCC_GPIOE_CLK_ENABLE
    __HAL_RCC_GPIOE_CLK_ENABLE();
#endif
#ifdef __HAL_RCC_GPIOF_CLK_ENABLE
    __HAL_RCC_GPIOF_CLK_ENABLE();
#endif
#ifdef __HAL_RCC_GPIOG_CLK_ENABLE
    __HAL_RCC_GPIOG_CLK_ENABLE();
#endif

    memset(&gpio, 0, sizeof(gpio));
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    gpio.Pin = BCM_DOOR_FL_Pin;
    HAL_GPIO_Init(BCM_DOOR_FL_GPIO_Port, &gpio);
    gpio.Pin = BCM_DOOR_FR_Pin;
    HAL_GPIO_Init(BCM_DOOR_FR_GPIO_Port, &gpio);
    gpio.Pin = BCM_DOOR_RL_Pin;
    HAL_GPIO_Init(BCM_DOOR_RL_GPIO_Port, &gpio);
    gpio.Pin = BCM_DOOR_RR_Pin;
    HAL_GPIO_Init(BCM_DOOR_RR_GPIO_Port, &gpio);
    gpio.Pin = BCM_TURN_LEFT_Pin;
    HAL_GPIO_Init(BCM_TURN_LEFT_GPIO_Port, &gpio);
    gpio.Pin = BCM_TURN_RIGHT_Pin;
    HAL_GPIO_Init(BCM_TURN_RIGHT_GPIO_Port, &gpio);
    gpio.Pin = BCM_HIGH_BEAM_Pin;
    HAL_GPIO_Init(BCM_HIGH_BEAM_GPIO_Port, &gpio);
    gpio.Pin = BCM_FOG_LAMP_Pin;
    HAL_GPIO_Init(BCM_FOG_LAMP_GPIO_Port, &gpio);

    memset((void *)&s_state, 0, sizeof(s_state));
    memset(&s_left_button, 0, sizeof(s_left_button));
    memset(&s_right_button, 0, sizeof(s_right_button));
}

void BCM_Input_Poll(void)
{
    BcmInput_State_t state = s_state;

    state.door_fl = read_input(BCM_DOOR_FL_GPIO_Port, BCM_DOOR_FL_Pin);
    state.door_fr = read_input(BCM_DOOR_FR_GPIO_Port, BCM_DOOR_FR_Pin);
    state.door_rl = read_input(BCM_DOOR_RL_GPIO_Port, BCM_DOOR_RL_Pin);
    state.door_rr = read_input(BCM_DOOR_RR_GPIO_Port, BCM_DOOR_RR_Pin);
    state.high_beam = read_input(BCM_HIGH_BEAM_GPIO_Port, BCM_HIGH_BEAM_Pin);
    state.fog_light = read_input(BCM_FOG_LAMP_GPIO_Port, BCM_FOG_LAMP_Pin);

    if (debounce_pressed(&s_left_button,
                         read_input(BCM_TURN_LEFT_GPIO_Port, BCM_TURN_LEFT_Pin))) {
        state.turn_left_enabled = state.turn_left_enabled ? 0U : 1U;
    }

    if (debounce_pressed(&s_right_button,
                         read_input(BCM_TURN_RIGHT_GPIO_Port, BCM_TURN_RIGHT_Pin))) {
        state.turn_right_enabled = state.turn_right_enabled ? 0U : 1U;
    }

    s_state = state;
}

void BCM_Input_GetState(BcmInput_State_t *out_state)
{
    if (out_state != NULL) {
        *out_state = s_state;
    }
}
