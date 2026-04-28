/**
 * @file    bcm_main.c
 * @brief   BCM FreeRTOS task orchestration.
 */

#include "bcm_body.h"
#include "bcm_can.h"
#include "bcm_input.h"
#include "bcm_signal.h"
#include "cmsis_os.h"
#include "uart.h"

/*
 * Stage switch:
 *   0 = standalone bench mode, send 0x470 every 100 ms immediately.
 *   1 = integration mode, send 0x470 only after Engine ECU 0x300 IGN ON.
 */
#ifndef BCM_BODY_WAIT_FOR_IGN
#define BCM_BODY_WAIT_FOR_IGN 0
#endif

#ifndef PERIOD_TURN_BLINK_MS
#define PERIOD_TURN_BLINK_MS 500U
#endif

#ifndef PERIOD_BODY_STATUS_MS
#define PERIOD_BODY_STATUS_MS 100U
#endif

static volatile uint8_t s_initialized;
static osThreadId_t s_input_task_handle;
static osThreadId_t s_ign_rx_task_handle;
static uint8_t s_input_log_valid;
static BcmInput_State_t s_last_logged_input;

static const osThreadAttr_t s_input_task_attributes = {
    .name = "bcmInput",
    .stack_size = 512U * 4U,
    .priority = (osPriority_t)osPriorityLow,
};

static const osThreadAttr_t s_ign_rx_task_attributes = {
    .name = "bcmIgnRx",
    .stack_size = 512U * 4U,
    .priority = (osPriority_t)osPriorityLow,
};

static uint8_t should_transmit(void)
{
#if BCM_BODY_WAIT_FOR_IGN
    return BCM_Can_IsIgnOn();
#else
    return 1U;
#endif
}

static uint8_t input_changed(const BcmInput_State_t *a, const BcmInput_State_t *b)
{
    return a->door_fl != b->door_fl ||
           a->door_fr != b->door_fr ||
           a->door_rl != b->door_rl ||
           a->door_rr != b->door_rr ||
           a->turn_left_enabled != b->turn_left_enabled ||
           a->turn_right_enabled != b->turn_right_enabled ||
           a->high_beam != b->high_beam ||
           a->fog_light != b->fog_light;
}

static void log_input_if_changed(const BcmInput_State_t *input)
{
    if (!s_input_log_valid || input_changed(input, &s_last_logged_input)) {
        uartPrintf(0,
                   "[BCM] INPUT door=%u%u%u%u turn=L%u/R%u high=%u fog=%u\r\n",
                   input->door_fl,
                   input->door_fr,
                   input->door_rl,
                   input->door_rr,
                   input->turn_left_enabled,
                   input->turn_right_enabled,
                   input->high_beam,
                   input->fog_light);
        s_last_logged_input = *input;
        s_input_log_valid = 1U;
    }
}

void BCM_Body_Init(void)
{
    if (s_initialized) {
        return;
    }

    BCM_Input_Init();
    if (!uartInit()) {
        return;
    }
    (void)BCM_Can_Init();
    s_initialized = 1U;

    uartPrintf(0, "[BCM] Body module ready, wait_for_ign=%u\r\n",
               (unsigned)BCM_BODY_WAIT_FOR_IGN);
}

void BCM_Body_InputTask(void *argument)
{
    (void)argument;

    for (;;) {
        BCM_Input_Poll();
        osDelay(20U);
    }
}

void BCM_Body_IgnRxTask(void *argument)
{
    (void)argument;

    for (;;) {
        BCM_Can_PollRx(100U);
    }
}

void BCM_Body_Task(void *argument)
{
    (void)argument;

    BCM_Body_Init();

    for (;;) {
        BcmSignal_BodyStatus_t status;
        CAN_Msg_t msg;
        uint32_t now = HAL_GetTick();
        uint8_t blink_on = ((now / PERIOD_TURN_BLINK_MS) % 2U) == 0U ? 1U : 0U;

        BCM_Input_GetState(&status.input);
        log_input_if_changed(&status.input);
        status.left_blink_on = blink_on;
        status.right_blink_on = blink_on;
        BCM_Signal_BuildBodyStatus(&status, &msg);

        if (should_transmit()) {
            (void)BCM_Can_SendBodyStatus(&msg);
        }

        osDelay(PERIOD_BODY_STATUS_MS);
    }
}

void BCM_Body_OnCanRx(const CAN_Msg_t *msg)
{
    BCM_Can_OnRx(msg);
}

uint8_t BCM_Body_IsIgnOn(void)
{
    return BCM_Can_IsIgnOn();
}

uint8_t BCM_Body_GetLampStatus(void)
{
    BcmInput_State_t input;
    uint8_t lamp = 0U;

    BCM_Input_GetState(&input);
    if (input.turn_left_enabled) {
        lamp |= BODY_BIT_TURN_LEFT;
    }
    if (input.turn_right_enabled) {
        lamp |= BODY_BIT_TURN_RIGHT;
    }
    if (input.high_beam) {
        lamp |= BODY_BIT_HIGH_BEAM;
    }
    if (input.fog_light) {
        lamp |= BODY_BIT_FOG_LAMP;
    }
    return lamp;
}

uint8_t BCM_Body_GetDoorStatus(void)
{
    BcmInput_State_t input;
    uint8_t door = 0U;

    BCM_Input_GetState(&input);
    if (input.door_fl) {
        door |= BODY_BIT_DOOR_FL;
    }
    if (input.door_fr) {
        door |= BODY_BIT_DOOR_FR;
    }
    if (input.door_rl) {
        door |= BODY_BIT_DOOR_RL;
    }
    if (input.door_rr) {
        door |= BODY_BIT_DOOR_RR;
    }
    return door;
}

uint32_t BCM_Body_GetTxCount(void)
{
    BcmCan_Stats_t stats;

    BCM_Can_GetStats(&stats);
    return stats.tx_count;
}

uint32_t BCM_Body_GetRxCount(void)
{
    BcmCan_Stats_t stats;

    BCM_Can_GetStats(&stats);
    return stats.rx_count;
}

/*
 * The current CubeMX FreeRTOS file creates EngineSim_Task unconditionally.
 * For the BCM build role this strong symbol reuses that slot as BodyTxTask,
 * then starts the separate InputTask.
 */
void EngineSim_Task(void *argument)
{
    BCM_Body_Init();

    if (s_input_task_handle == NULL) {
        s_input_task_handle = osThreadNew(BCM_Body_InputTask, NULL, &s_input_task_attributes);
    }
    if (s_ign_rx_task_handle == NULL) {
        s_ign_rx_task_handle = osThreadNew(BCM_Body_IgnRxTask, NULL, &s_ign_rx_task_attributes);
    }

    BCM_Body_Task(argument);
}
