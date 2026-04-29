#include "can_bsp.h"

extern CAN_HandleTypeDef hcan1;
#ifdef BOARD_B_GATEWAY
extern CAN_HandleTypeDef hcan2;
#endif

osMessageQueueId_t can_rx_q = NULL;

volatile uint32_t can1TxCount = 0;
volatile uint32_t can1RxCount = 0;
volatile uint32_t can2TxCount = 0;
volatile uint32_t can2RxCount = 0;
volatile uint32_t canSendEnterCount = 0;
volatile uint32_t canTxBusyCount = 0;
volatile uint32_t canTxErrorCount = 0;
volatile uint32_t canInitStep = 0;
volatile uint32_t canInitErrorStep = 0;

#define CAN_RX_Q_SIZE 64U

HAL_StatusTypeDef CAN_BSP_Init(void)
{
    canInitStep = 1U;
    canInitErrorStep = 0U;

    if (can_rx_q == NULL) {
        can_rx_q = osMessageQueueNew(CAN_RX_Q_SIZE, sizeof(CAN_RxMessage_t), NULL);
        if (can_rx_q == NULL) {
            canInitErrorStep = 1U;
            return HAL_ERROR;
        }
    }

    canInitStep = 2U;

    CAN_FilterTypeDef filter1 = {0};
    filter1.FilterActivation = ENABLE;
    filter1.FilterBank = 0;
    filter1.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter1.FilterIdHigh = 0x0000;
    filter1.FilterIdLow = 0x0000;
    filter1.FilterMaskIdHigh = 0x0000;
    filter1.FilterMaskIdLow = 0x0000;
    filter1.FilterMode = CAN_FILTERMODE_IDMASK;
    filter1.FilterScale = CAN_FILTERSCALE_32BIT;
    filter1.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &filter1) != HAL_OK) {
        canInitErrorStep = 2U;
        return HAL_ERROR;
    }

    canInitStep = 3U;

    if (HAL_CAN_Start(&hcan1) != HAL_OK) {
        canInitErrorStep = 3U;
        return HAL_ERROR;
    }

    canInitStep = 4U;

    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        canInitErrorStep = 4U;
        return HAL_ERROR;
    }

#ifdef BOARD_B_GATEWAY
    canInitStep = 5U;

    CAN_FilterTypeDef filter2 = {0};
    filter2.FilterActivation = ENABLE;
    filter2.FilterBank = 14;
    filter2.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter2.FilterIdHigh = 0x0000;
    filter2.FilterIdLow = 0x0000;
    filter2.FilterMaskIdHigh = 0x0000;
    filter2.FilterMaskIdLow = 0x0000;
    filter2.FilterMode = CAN_FILTERMODE_IDMASK;
    filter2.FilterScale = CAN_FILTERSCALE_32BIT;
    filter2.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan2, &filter2) != HAL_OK) {
        canInitErrorStep = 5U;
        return HAL_ERROR;
    }

    canInitStep = 6U;

    if (HAL_CAN_Start(&hcan2) != HAL_OK) {
        canInitErrorStep = 6U;
        return HAL_ERROR;
    }

    canInitStep = 7U;

    if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        canInitErrorStep = 7U;
        return HAL_ERROR;
    }
#endif

    canInitStep = 8U;
    return HAL_OK;
}

HAL_StatusTypeDef CAN_BSP_SendTo(CAN_HandleTypeDef *hcan, uint32_t id, uint8_t *data, uint8_t len)
{
    CAN_TxHeaderTypeDef txHeader = {0};
    uint32_t txMailbox = 0;

    canSendEnterCount++;

    if (data == NULL || len > 8U) {
        canTxErrorCount++;
        return HAL_ERROR;
    }

    if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0U) {
        canTxBusyCount++;
        return HAL_BUSY;
    }

    txHeader.StdId = id;
    txHeader.DLC = len;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.TransmitGlobalTime = DISABLE;

    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hcan, &txHeader, data, &txMailbox);
    if (status == HAL_OK) {
        if (hcan->Instance == CAN1) {
            can1TxCount++;
        }
#ifdef BOARD_B_GATEWAY
        else if (hcan->Instance == CAN2) {
            can2TxCount++;
        }
#endif
    } else {
        canTxErrorCount++;
    }

    return status;
}

HAL_StatusTypeDef CAN_BSP_Send(uint32_t id, uint8_t *data, uint8_t len)
{
    return CAN_BSP_SendTo(&hcan1, id, data, len);
}

bool CAN_BSP_Read(CAN_RxMessage_t *p_msg, uint32_t timeout)
{
    if (can_rx_q == NULL) {
        return false;
    }

    return osMessageQueueGet(can_rx_q, p_msg, NULL, timeout) == osOK;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rxHeader = {0};
    CAN_RxMessage_t rxMsg = {0};

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxMsg.data) != HAL_OK) {
        return;
    }

    rxMsg.bus = (hcan->Instance == CAN1) ? 1U : 2U;
    rxMsg.id = rxHeader.StdId;
    rxMsg.dlc = rxHeader.DLC;

    if (hcan->Instance == CAN1) {
        can1RxCount++;
    }
#ifdef BOARD_B_GATEWAY
    else if (hcan->Instance == CAN2) {
        can2RxCount++;
    }
#endif

    if (can_rx_q != NULL) {
        (void)osMessageQueuePut(can_rx_q, &rxMsg, 0U, 0U);
    }
}
