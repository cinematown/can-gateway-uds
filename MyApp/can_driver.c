#include "can_driver.h"

extern CAN_HandleTypeDef hcan1;

static CAN_TxHeaderTypeDef txHeader;
static uint32_t txMailbox;

volatile uint32_t can1TxCount = 0;
volatile uint32_t canSendEnterCount = 0;
volatile uint32_t canTxBusyCount = 0;
volatile uint32_t canTxErrorCount = 0;

HAL_StatusTypeDef CAN_Driver_Init(void)
{
    CAN_FilterTypeDef filter = {0};

    filter.FilterActivation = ENABLE;
    filter.FilterBank = 0;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterIdHigh = 0x0000;
    filter.FilterIdLow = 0x0000;
    filter.FilterMaskIdHigh = 0x0000;
    filter.FilterMaskIdLow = 0x0000;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        return HAL_ERROR;
    }

    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.TransmitGlobalTime = DISABLE;

    return HAL_OK;
}

HAL_StatusTypeDef CAN1_Send(uint32_t id, uint8_t *data, uint8_t len)
{
    canSendEnterCount++;

    if (data == NULL)
    {
        canTxErrorCount++;
        return HAL_ERROR;
    }

    if (len > 8)
    {
        canTxErrorCount++;
        return HAL_ERROR;
    }

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)
    {
        canTxBusyCount++;
        return HAL_BUSY;
    }

    txHeader.StdId = id;
    txHeader.DLC = len;

    HAL_StatusTypeDef ret =
        HAL_CAN_AddTxMessage(&hcan1, &txHeader, data, &txMailbox);

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