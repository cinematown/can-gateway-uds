#include "can_bsp.h"

// 1. CAN 하드웨어 시작 및 필터 설정
void CAN_BSP_Init(void) {
    // CAN 필터 설정 (기본적으로 모든 메시지 수신 또는 특정 ID 설정)
    CAN_BSP_ConfigFilter_UDS_Response(); 

    // CAN 컨트롤러 시작
    HAL_CAN_Start(&hcan1);

    // 인터럽트 방식 수신 활성화 (필요한 경우)
    //HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}
void CAN_BSP_ConfigFilter_Open(void) {
    CAN_FilterTypeDef sFilterConfig;

    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    
    // ID와 Mask를 모두 0으로 설정하면 모든 메시지를 수신합니다.
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000; // 0으로 설정 시 모든 비트 무시 (Pass All)
    sFilterConfig.FilterMaskIdLow = 0x0000;
    
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
}
// 2. 필터 설정 (에러 방지용 기본 구현)
void CAN_BSP_ConfigFilter(uint32_t id) {
    CAN_FilterTypeDef sFilterConfig;

    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = (id << 5);
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0xFFE0;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
}
// can_bsp.c - 필터 함수 수정
void CAN_BSP_ConfigFilter_UDS_Response(void) {
    CAN_FilterTypeDef sFilterConfig;
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    
    // 0x77E만 통과
    sFilterConfig.FilterIdHigh = (CAN_ID_UDS_RESP_BOARD_B << 5);   // ID를 왼쪽으로 5비트 시프트
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0xFFE0;     // 11비트 ID 전체 비교
    sFilterConfig.FilterMaskIdLow = 0x0000;
    
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;
    HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
}
// 3. [중요!] 에러의 주인공 - 수신 메시지 가져오기 함수
HAL_StatusTypeDef CAN_BSP_GetRxMessage(CAN_RxMessage_t *pMsg) {
    // FIFO0에 메시지가 쌓여 있는지 확인
    if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) {
        // 메시지 읽어오기
        if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &pMsg->header, pMsg->data) == HAL_OK) {
            pMsg->id = pMsg->header.StdId;
            pMsg->dlc = pMsg->header.DLC;
            return HAL_OK;
        }
    }
    return HAL_ERROR;
}
/* UDSAPP/can_bsp.c */

/* can_bsp.c */
/* can_bsp.c 또는 uds_service.c */
// 기존: void UDS_Request_ReadData(uint16_t dataId)  <-- X
// 변경: 
void UDS_Request_ReadData(uint32_t canId, uint16_t did) // <-- Header와 동일하게 수정
{
    CAN_TxHeaderTypeDef txHeader;
    uint8_t txData[8] = {0,};
    uint32_t txMailbox;

    txHeader.StdId = canId;  // 이제 고정값이 아니라 인자로 받은 ID 사용!
    txHeader.DLC = 8;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;

    txData[0] = 0x03; 
    txData[1] = 0x22; 
    txData[2] = (uint8_t)(did >> 8);
    txData[3] = (uint8_t)(did & 0xFF);

    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox);
    if (status != HAL_OK) {
        cliPrintf("[CAN TX Error] Code: %d\r\n", status);
    }
}
