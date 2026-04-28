#include "can_bsp.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

osMessageQueueId_t can_rx_q = NULL;

volatile uint32_t can1TxCount = 0;
volatile uint32_t can1RxCount = 0;
volatile uint32_t can2TxCount = 0;
volatile uint32_t can2RxCount = 0;
volatile uint32_t canSendEnterCount = 0;
volatile uint32_t canTxBusyCount = 0;
volatile uint32_t canTxErrorCount = 0;

#define CAN_RX_Q_SIZE 64

static HAL_StatusTypeDef CAN_BSP_ConfigFilterFor(CAN_HandleTypeDef *hcan,
                                                 uint32_t bank,
                                                 uint32_t fifo,
                                                 uint32_t std_id,
                                                 bool accept_all)
{
    CAN_FilterTypeDef filter = {0};

    filter.FilterActivation = ENABLE;
    filter.FilterBank = bank;
    filter.FilterFIFOAssignment = fifo;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.SlaveStartFilterBank = 14;

    if (accept_all) {
        filter.FilterIdHigh = 0x0000;
        filter.FilterIdLow = 0x0000;
        filter.FilterMaskIdHigh = 0x0000;
        filter.FilterMaskIdLow = 0x0000;
    } else {
        filter.FilterIdHigh = (uint16_t)(std_id << 5);
        filter.FilterIdLow = 0x0000;
        filter.FilterMaskIdHigh = 0xFFE0;
        filter.FilterMaskIdLow = 0x0000;
    }

    return HAL_CAN_ConfigFilter(hcan, &filter);
}

HAL_StatusTypeDef CAN_BSP_Init(void)
{
    HAL_StatusTypeDef ret;

    if (can_rx_q == NULL) {
        can_rx_q = osMessageQueueNew(CAN_RX_Q_SIZE, sizeof(CAN_RxMessage_t), NULL);
    }

#ifdef BOARD_C_UDS
    ret = CAN_BSP_ConfigFilterFor(&hcan1, 0, CAN_RX_FIFO0, CAN_ID_UDS_RESP_BOARD_B, false);
#else
    ret = CAN_BSP_ConfigFilterFor(&hcan1, 0, CAN_RX_FIFO0, 0, true);
#endif
    if (ret != HAL_OK) {
        return ret;
    }

    ret = HAL_CAN_Start(&hcan1);
    if (ret != HAL_OK) {
        return ret;
    }

    ret = HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
    if (ret != HAL_OK) {
        return ret;
    }

#ifdef BOARD_B_GATEWAY
    ret = CAN_BSP_ConfigFilterFor(&hcan2, 14, CAN_RX_FIFO0, 0, true);
    if (ret != HAL_OK) {
        return ret;
    }

    ret = HAL_CAN_Start(&hcan2);
    if (ret != HAL_OK) {
        return ret;
    }

    ret = HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
    if (ret != HAL_OK) {
        return ret;
    }
#endif

    return HAL_OK;
}

HAL_StatusTypeDef CAN_BSP_SendTo(CAN_HandleTypeDef *hcan, uint32_t id, uint8_t *data, uint8_t len)
{
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;
    HAL_StatusTypeDef ret;

    canSendEnterCount++;

    if (hcan == NULL || data == NULL || len > 8) {
        canTxErrorCount++;
        return HAL_ERROR;
    }

    if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0) {
        canTxBusyCount++;
        return HAL_BUSY;
    }

    txHeader.StdId = id;
    txHeader.DLC = len;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.TransmitGlobalTime = DISABLE;

    ret = HAL_CAN_AddTxMessage(hcan, &txHeader, data, &txMailbox);
    if (ret == HAL_OK) {
        if (hcan->Instance == CAN2) {
            can2TxCount++;
        } else {
            can1TxCount++;
        }
    } else {
        canTxErrorCount++;
    }

    return ret;
}

HAL_StatusTypeDef CAN_BSP_Send(uint32_t id, uint8_t *data, uint8_t len)
{
    return CAN_BSP_SendTo(&hcan1, id, data, len);
}

bool CAN_BSP_Read(CAN_RxMessage_t *p_msg, uint32_t timeout)
{
    if (p_msg != NULL && can_rx_q != NULL &&
        osMessageQueueGet(can_rx_q, p_msg, NULL, timeout) == osOK) {
        return true;
    }

    return false;
}

void CAN_BSP_ConfigFilter_Open(void)
{
    (void)CAN_BSP_ConfigFilterFor(&hcan1, 0, CAN_RX_FIFO0, 0, true);
}

void CAN_BSP_ConfigFilter(uint32_t id)
{
    (void)CAN_BSP_ConfigFilterFor(&hcan1, 0, CAN_RX_FIFO0, id, false);
}

void CAN_BSP_ConfigFilter_UDS_Response(void)
{
    CAN_BSP_ConfigFilter(CAN_ID_UDS_RESP_BOARD_B);
}

HAL_StatusTypeDef CAN_BSP_GetRxMessage(CAN_RxMessage_t *p_msg)
{
    if (p_msg == NULL) {
        return HAL_ERROR;
    }

    if (CAN_BSP_Read(p_msg, 0)) {
        return HAL_OK;
    }

    if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) {
        if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &p_msg->header, p_msg->data) == HAL_OK) {
            p_msg->bus = 1;
            p_msg->id = p_msg->header.StdId;
            p_msg->dlc = p_msg->header.DLC;
            return HAL_OK;
        }
    }

    return HAL_ERROR;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxMessage_t rxMsg = {0};

    if (hcan == NULL) {
        return;
    }

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxMsg.header, rxMsg.data) == HAL_OK) {
        rxMsg.bus = (hcan->Instance == CAN2) ? 2 : 1;
        rxMsg.id = rxMsg.header.StdId;
        rxMsg.dlc = rxMsg.header.DLC;

        if (rxMsg.bus == 2) {
            can2RxCount++;
        } else {
            can1RxCount++;
        }

        if (can_rx_q != NULL) {
            (void)osMessageQueuePut(can_rx_q, &rxMsg, 0, 0);
        }
    }
}
