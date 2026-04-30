#include "can_bsp.h"
#include <stddef.h>

/*
 * Board C는 실제 계기판/진단 버스를 CAN1로 사용합니다.
 * CubeMX에서 CAN1이 활성화되어 있어야 하며, generated code 쪽에 hcan1이 있어야 합니다.
 * 현재 STM32F429ZI CubeMX 설정 기준 CAN1 핀:
 * - CAN1_RX = PD0
 * - CAN1_TX = PD1
 */
extern CAN_HandleTypeDef hcan1;

osMessageQueueId_t can_rx_q = NULL;

volatile uint32_t can1TxCount        = 0u;
volatile uint32_t can1RxCount        = 0u;
volatile uint32_t canSendEnterCount  = 0u;
volatile uint32_t canTxBusyCount     = 0u;
volatile uint32_t canTxErrorCount    = 0u;

#define CAN_RX_Q_SIZE 64u

HAL_StatusTypeDef CAN_BSP_Init(void)
{
    if (can_rx_q == NULL)
    {
        can_rx_q = osMessageQueueNew(CAN_RX_Q_SIZE, sizeof(CAN_RxMessage_t), NULL);
        if (can_rx_q == NULL)
        {
            return HAL_ERROR;
        }
    }

    /*
     * CAN1 수신 필터 설정
     * - 모든 Standard ID 수신
     * - FIFO0로 수신하고 HAL_CAN_RxFifo0MsgPendingCallback()에서 큐에 저장
     */
    CAN_FilterTypeDef filter1 = {0};
    filter1.FilterActivation     = ENABLE;
    filter1.FilterBank           = 0u;
    filter1.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter1.FilterIdHigh         = 0x0000u;
    filter1.FilterIdLow          = 0x0000u;
    filter1.FilterMaskIdHigh     = 0x0000u;
    filter1.FilterMaskIdLow      = 0x0000u;
    filter1.FilterMode           = CAN_FILTERMODE_IDMASK;
    filter1.FilterScale          = CAN_FILTERSCALE_32BIT;
    filter1.SlaveStartFilterBank = 14u;

    if (HAL_CAN_ConfigFilter(&hcan1, &filter1) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef CAN_BSP_SendTo(CAN_HandleTypeDef *hcan, uint32_t id, uint8_t *data, uint8_t len)
{
    CAN_TxHeaderTypeDef txHeader = {0};
    uint32_t txMailbox = 0u;
    HAL_StatusTypeDef ret;

    canSendEnterCount++;

    if (hcan == NULL || data == NULL || len > 8u)
    {
        canTxErrorCount++;
        return HAL_ERROR;
    }

    if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0u)
    {
        canTxBusyCount++;
        return HAL_BUSY;
    }

    txHeader.StdId              = id;
    txHeader.DLC                = len;
    txHeader.IDE                = CAN_ID_STD;
    txHeader.RTR                = CAN_RTR_DATA;
    txHeader.TransmitGlobalTime = DISABLE;

    ret = HAL_CAN_AddTxMessage(hcan, &txHeader, data, &txMailbox);
    if (ret == HAL_OK)
    {
        can1TxCount++;
    }
    else
    {
        canTxErrorCount++;
    }

    return ret;
}

/*
 * Board C 기본 송신 채널은 CAN1입니다.
 * 계기판 UDS 요청 0x714도 이 함수를 통해 CAN1로 송신됩니다.
 */
HAL_StatusTypeDef CAN_BSP_Send(uint32_t id, uint8_t *data, uint8_t len)
{
    return CAN_BSP_SendTo(&hcan1, id, data, len);
}

bool CAN_BSP_Read(CAN_RxMessage_t *p_msg, uint32_t timeout)
{
    if (p_msg == NULL || can_rx_q == NULL)
    {
        return false;
    }

    return (osMessageQueueGet(can_rx_q, p_msg, NULL, timeout) == osOK);
}

/* CAN1 RX FIFO0 interrupt callback */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rxHeader = {0};
    CAN_RxMessage_t rxMsg = {0};

    if (hcan == NULL || hcan->Instance != CAN1)
    {
        return;
    }

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxMsg.data) == HAL_OK)
    {
        rxMsg.bus = CAN_BSP_BUS_CAN1;
        rxMsg.id  = rxHeader.StdId;
        rxMsg.dlc = rxHeader.DLC;
        can1RxCount++;

        if (can_rx_q != NULL)
        {
            (void)osMessageQueuePut(can_rx_q, &rxMsg, 0u, 0u);
        }
    }
}
