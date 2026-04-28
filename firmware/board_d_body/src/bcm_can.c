/**
 * @file    bcm_can.c
 * @brief   BCM CAN transmit and receive handling through common CAN BSP.
 */

#include "bcm_can.h"
#include "can_bsp.h"
#include "can.h"
#include "protocol_ids.h"
#include "uart.h"
#include <string.h>

extern CAN_HandleTypeDef hcan1;

static volatile uint8_t s_ign_on;
static volatile BcmCan_Stats_t s_stats;

int BCM_Can_Init(void)
{
    memset((void *)&s_stats, 0, sizeof(s_stats));
    s_ign_on = 0U;

    if (CAN_BSP_Init() != HAL_OK) {
        uartPrintf(0, "[BCM] CAN BSP init failed\r\n");
        return -1;
    }

    uartPrintf(0, "[BCM] CAN BSP started on CAN1\r\n");
    return 0;
}

int BCM_Can_SendBodyStatus(const CAN_Msg_t *msg)
{
    if (msg == NULL || msg->dlc > 8U) {
        return -1;
    }

    if (CAN_BSP_Send(msg->id, (uint8_t *)msg->data, msg->dlc) != HAL_OK) {
        s_stats.tx_error_count++;
        uartPrintf(0, "[BCM] TX FAIL id=0x%03lX tx=%lu err=%lu hal=0x%08lX\r\n",
                   (unsigned long)msg->id,
                   (unsigned long)s_stats.tx_count,
                   (unsigned long)s_stats.tx_error_count,
                   (unsigned long)HAL_CAN_GetError(&hcan1));
        return -1;
    }

    s_stats.tx_count++;
    if ((s_stats.tx_count % 10U) == 0U) {
        uartPrintf(0,
                   "[BCM] TX REQ id=0x%03lX tx=%lu data=%02X %02X %02X %02X %02X %02X %02X %02X hal=0x%08lX\r\n",
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

    if (msg == NULL) {
        return;
    }

    s_stats.rx_count++;

    if (msg->id != CAN_ID_IGN_STATUS || msg->dlc < CAN_DLC_IGN_STATUS) {
        return;
    }

    prev_ign = s_ign_on;
    s_ign_on = VW300_GET_IGN_ON(msg->data) ? 1U : 0U;

    if (prev_ign != s_ign_on) {
        uartPrintf(0, "[BCM] IGN %s\r\n", s_ign_on ? "ON" : "OFF");
    }
}

uint8_t BCM_Can_IsIgnOn(void)
{
    return s_ign_on;
}

void BCM_Can_GetStats(BcmCan_Stats_t *out_stats)
{
    if (out_stats != NULL) {
        *out_stats = s_stats;
    }
}
