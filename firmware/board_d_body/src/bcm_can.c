/**
 * @file    bcm_can.c
 * @brief   BCM CAN transmit and receive handling through common CAN BSP.
 */

#include "bcm_can.h"
#include "bcm_signal.h"
#include "can_bsp.h"
#include "can.h"
#include "protocol_ids.h"
#include "uart.h"
#include <string.h>

extern CAN_HandleTypeDef hcan1;

/* If the engine CAN frame stops arriving, treat IGN as OFF after this delay. */
#ifndef BCM_IGN_TIMEOUT_MS
#define BCM_IGN_TIMEOUT_MS 500U
#endif

#define GOLF6_CAN_ID_BSG_LAST        0x570U
#define GOLF6_CAN_ID_ZAS_1           0x572U
#define GOLF6_KLEMME_15_BIT          1U

static volatile uint8_t s_ign_on;
static volatile uint8_t s_ign_frame_valid;
static volatile uint32_t s_last_ign_rx_tick;
static volatile BcmCan_Stats_t s_stats;

static uint8_t get_bit(const uint8_t *data, uint8_t bit)
{
    return (data[bit / 8U] & (uint8_t)(1U << (bit % 8U))) != 0U;
}

static uint8_t decode_ign_on(const CAN_Msg_t *msg, const char **source)
{
    if (msg->id == CAN_ID_ENGINE_DATA && msg->dlc == CAN_ENGINE_DATA_DLC) {
        *source = "ENGINE_DATA";
        return VW300_GET_IGN_ON(msg->data) ? 1U : 0U;
    }

    if ((msg->id == GOLF6_CAN_ID_ZAS_1 || msg->id == GOLF6_CAN_ID_BSG_LAST) &&
        msg->dlc >= 1U) {
        *source = (msg->id == GOLF6_CAN_ID_ZAS_1) ? "mZAS_1" : "mBSG_Last";
        return get_bit(msg->data, GOLF6_KLEMME_15_BIT) ? 1U : 0U;
    }

    *source = NULL;
    return 0U;
}

int BCM_Can_Init(void)
{
    memset((void *)&s_stats, 0, sizeof(s_stats));
    s_ign_on = 0U;
    s_ign_frame_valid = 0U;
    s_last_ign_rx_tick = 0U;

    if (CAN_BSP_Init() != HAL_OK) {
        uartPrintf(0, "[BCM] CAN BSP init failed\r\n");
        return -1;
    }

    uartPrintf(0, "[BCM] CAN BSP started on CAN1\r\n");
    return 0;
}

int BCM_Can_SendBodyStatus(const CAN_Msg_t *msg)
{
    HAL_StatusTypeDef tx_status;

    if (msg == NULL ||
        msg->id != BCM_GOLF6_CAN_ID_MGATE_KOMF_1 ||
        msg->dlc != BCM_GOLF6_MGATE_KOMF_1_DLC) {
        s_stats.tx_error_count++;
        uartPrintf(0, "[BCM] TX REJECT id=0x%03lX dlc=%u err=%lu\r\n",
                   msg != NULL ? (unsigned long)msg->id : 0UL,
                   msg != NULL ? (unsigned int)msg->dlc : 0U,
                   (unsigned long)s_stats.tx_error_count);
        return -1;
    }

    tx_status = CAN_BSP_SendTo(&hcan1, msg->id, (uint8_t *)msg->data, msg->dlc);
    if (tx_status != HAL_OK) {
        s_stats.tx_error_count++;
        uartPrintf(0, "[BCM] CAN1 TX FAIL id=0x%03lX tx=%lu err=%lu st=%d hal=0x%08lX\r\n",
                   (unsigned long)msg->id,
                   (unsigned long)s_stats.tx_count,
                   (unsigned long)s_stats.tx_error_count,
                   (int)tx_status,
                   (unsigned long)HAL_CAN_GetError(&hcan1));
        return -1;
    }

    s_stats.tx_count++;
    if (s_stats.tx_count == 1U || (s_stats.tx_count % 10U) == 0U) {
        uartPrintf(0,
                   "[BCM] CAN1 TX OK id=0x%03lX tx=%lu data=%02X %02X %02X %02X %02X %02X %02X %02X hal=0x%08lX\r\n",
                   (unsigned long)msg->id,
                   (unsigned long)s_stats.tx_count,
                   msg->data[0], msg->data[1], msg->data[2], msg->data[3],
                   msg->data[4], msg->data[5], msg->data[6], msg->data[7],
                   (unsigned long)HAL_CAN_GetError(&hcan1));
    }
    return 0;
}

void BCM_Can_PollRx(uint32_t timeout_ms)
{
    CAN_RxMessage_t rx;
    CAN_Msg_t msg;

    if (!CAN_BSP_Read(&rx, timeout_ms)) {
        return;
    }

    memset(&msg, 0, sizeof(msg));
    msg.id = rx.id;
    msg.dlc = rx.dlc;
    memcpy(msg.data, rx.data, sizeof(msg.data));
    BCM_Can_OnRx(&msg);
}

void BCM_Can_OnRx(const CAN_Msg_t *msg)
{
    uint8_t prev_ign;
    const char *ign_source;
    uint8_t ign_on;

    if (msg == NULL) {
        return;
    }

    s_stats.rx_count++;

    ign_on = decode_ign_on(msg, &ign_source);
    if (ign_source == NULL) {
        return;
    }

    prev_ign = s_ign_on;
    s_ign_on = ign_on;
    s_ign_frame_valid = 1U;
    s_last_ign_rx_tick = HAL_GetTick();

    if (prev_ign != s_ign_on) {
        uartPrintf(0, "[BCM] IGN %s via %s (id=0x%03lX)\r\n",
                   s_ign_on ? "ON" : "OFF",
                   ign_source,
                   (unsigned long)msg->id);
    }
}

uint8_t BCM_Can_IsIgnOn(void)
{
    uint32_t now;

    if (!s_ign_frame_valid || !s_ign_on) {
        return 0U;
    }

    now = HAL_GetTick();
    if ((uint32_t)(now - s_last_ign_rx_tick) > BCM_IGN_TIMEOUT_MS) {
        return 0U;
    }

    return s_ign_on;
}

void BCM_Can_GetStats(BcmCan_Stats_t *out_stats)
{
    if (out_stats != NULL) {
        *out_stats = s_stats;
    }
}
