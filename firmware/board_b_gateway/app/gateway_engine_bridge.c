#include "gateway_engine_bridge.h"

#include "can_cli_monitor.h"

#include <stdbool.h>
#include <stdint.h>

extern CAN_HandleTypeDef hcan2;

#define CLUSTER_CAN_ID_MOTOR_1        0x280U
#define CLUSTER_CAN_ID_BREMSE_1       0x1A0U

#define CLUSTER_MOTOR_1_PERIOD_MS     50U
#define CLUSTER_BREMSE_1_PERIOD_MS    500U
#define CLUSTER_INPUT_TIMEOUT_MS      1000U

#define CLUSTER_MOTOR_1_RPM_START     16U
#define CLUSTER_MOTOR_1_RPM_LENGTH    16U
#define CLUSTER_BREMSE_1_SPEED_START  17U
#define CLUSTER_BREMSE_1_SPEED_LENGTH 15U

typedef struct {
    uint16_t rpm;
    uint16_t speed_kmh;
    uint8_t coolant_c;
    uint8_t board_a_alive;
    bool ign_on;
    uint32_t last_rx_tick;
} ClusterInputState_t;

static ClusterInputState_t s_cluster_input = {0};
static uint8_t s_bremse_alive_counter = 0U;

static uint32_t tick_elapsed(uint32_t now, uint32_t before)
{
    return now - before;
}

static void clear_frame(uint8_t data[8])
{
    for (uint8_t i = 0U; i < 8U; i++) {
        data[i] = 0U;
    }
}

static void set_le_signal(uint8_t data[8], uint8_t start_bit,
                          uint8_t bit_len, uint32_t raw)
{
    for (uint8_t i = 0U; i < bit_len; i++) {
        uint8_t bit_pos = (uint8_t)(start_bit + i);
        uint8_t byte_idx = (uint8_t)(bit_pos / 8U);
        uint8_t bit_idx = (uint8_t)(bit_pos % 8U);

        if ((raw & (1UL << i)) != 0UL) {
            data[byte_idx] |= (uint8_t)(1U << bit_idx);
        } else {
            data[byte_idx] &= (uint8_t)~(1U << bit_idx);
        }
    }
}

static void update_input_from_board_a(const CAN_RxMessage_t *rx_msg)
{
    uint8_t status = rx_msg->data[CAN_ENGINE_DATA_STATUS_IDX];

    s_cluster_input.rpm = CAN_GetU16LE(rx_msg->data, CAN_ENGINE_DATA_RPM_IDX);
    s_cluster_input.speed_kmh = CAN_GetU16LE(rx_msg->data, CAN_ENGINE_DATA_SPEED_IDX);
    s_cluster_input.coolant_c = rx_msg->data[CAN_ENGINE_DATA_COOLANT_IDX];
    s_cluster_input.ign_on = ((status & CAN_ENGINE_STATUS_IGN_MASK) != 0U);
    s_cluster_input.board_a_alive =
        (uint8_t)((status & CAN_ENGINE_STATUS_ALIVE_MASK) >> CAN_ENGINE_STATUS_ALIVE_SHIFT);
    s_cluster_input.last_rx_tick = osKernelGetTickCount();
}

static bool is_input_active(uint32_t now)
{
    if (!s_cluster_input.ign_on) {
        return false;
    }

    return tick_elapsed(now, s_cluster_input.last_rx_tick) <= CLUSTER_INPUT_TIMEOUT_MS;
}

static uint16_t get_active_rpm(uint32_t now)
{
    return is_input_active(now) ? s_cluster_input.rpm : 0U;
}

static uint16_t get_active_speed(uint32_t now)
{
    return is_input_active(now) ? s_cluster_input.speed_kmh : 0U;
}

static void encode_motor_1(uint8_t data[8], uint16_t rpm)
{
    clear_frame(data);
    set_le_signal(data,
                  CLUSTER_MOTOR_1_RPM_START,
                  CLUSTER_MOTOR_1_RPM_LENGTH,
                  (uint32_t)rpm * 4U);
}

static void encode_bremse_1(uint8_t data[8], uint16_t speed_kmh)
{
    clear_frame(data);

    data[0] = 0x08U;
    set_le_signal(data,
                  CLUSTER_BREMSE_1_SPEED_START,
                  CLUSTER_BREMSE_1_SPEED_LENGTH,
                  (uint32_t)speed_kmh * 100U);

    data[7] = (uint8_t)((data[7] & 0xF0U) | (s_bremse_alive_counter & 0x0FU));
    s_bremse_alive_counter = (uint8_t)((s_bremse_alive_counter + 1U) & 0x0FU);
}

static void send_motor_1(uint32_t now)
{
    uint8_t data[8];

    encode_motor_1(data, get_active_rpm(now));
    HAL_StatusTypeDef status =
        CAN_BSP_SendTo(&hcan2, CLUSTER_CAN_ID_MOTOR_1, data, 8U);
    CanCliMonitor_LogTx(2U, CLUSTER_CAN_ID_MOTOR_1, data, 8U, status);
}

static void send_bremse_1(uint32_t now)
{
    uint8_t data[8];

    encode_bremse_1(data, get_active_speed(now));
    HAL_StatusTypeDef status =
        CAN_BSP_SendTo(&hcan2, CLUSTER_CAN_ID_BREMSE_1, data, 8U);
    CanCliMonitor_LogTx(2U, CLUSTER_CAN_ID_BREMSE_1, data, 8U, status);
}

void GatewayEngineBridge_OnRx(const CAN_RxMessage_t *rx_msg)
{
    if (rx_msg == NULL) {
        return;
    }

    if (rx_msg->bus == 1U &&
        rx_msg->id == CAN_ID_ENGINE_DATA &&
        rx_msg->dlc >= CAN_ENGINE_DATA_DLC) {
        update_input_from_board_a(rx_msg);
    }
}

void GatewayEngineBridge_Task10ms(void)
{
    static uint32_t next_motor_1_tick = 0U;
    static uint32_t next_bremse_1_tick = 0U;
    uint32_t now = osKernelGetTickCount();

    if (next_motor_1_tick == 0U) {
        next_motor_1_tick = now;
        next_bremse_1_tick = now;
    }

    if (tick_elapsed(now, next_motor_1_tick) < 0x80000000UL) {
        send_motor_1(now);
        next_motor_1_tick = now + CLUSTER_MOTOR_1_PERIOD_MS;
    }

    if (tick_elapsed(now, next_bremse_1_tick) < 0x80000000UL) {
        send_bremse_1(now);
        next_bremse_1_tick = now + CLUSTER_BREMSE_1_PERIOD_MS;
    }
}

void GatewayEngineBridge_GetState(GatewayEngineBridge_State_t *state)
{
    if (state == NULL) {
        return;
    }

    uint32_t now = osKernelGetTickCount();
    uint32_t age = tick_elapsed(now, s_cluster_input.last_rx_tick);

    state->rpm = s_cluster_input.rpm;
    state->speed_kmh = s_cluster_input.speed_kmh;
    state->coolant_c = s_cluster_input.coolant_c;
    state->board_a_alive = s_cluster_input.board_a_alive;
    state->ign_on = s_cluster_input.ign_on ? 1U : 0U;
    state->active = is_input_active(now) ? 1U : 0U;
    state->age_ms = age;
}
