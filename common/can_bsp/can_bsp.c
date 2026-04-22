/**
 * @file    can_bsp.c
 * @brief   CAN BSP Stub - 실제 구현 전 임시 동작 (UART 출력으로 TX 시뮬레이션)
 * @author  ① 성재, 민진
 * @note    【중요】이것은 Day 2 배포용 STUB 입니다.
 *          실제 CAN 송수신은 담당자가 구현해주세요.
 *          stub 덕분에 ②③④⑤ 팀원이 CAN 드라이버 완성을 기다리지 않고 개발 가능.
 *
 *   TODO(①):
 *     1. HAL_CAN_ConfigFilter()로 Filter Bank 설정
 *     2. HAL_CAN_Start(), HAL_CAN_ActivateNotification()
 *     3. HAL_CAN_AddTxMessage() 로 실제 TX
 *     4. HAL_CAN_RxFifo0MsgPendingCallback()에서 Queue push
 *     5. CAN1/CAN2 분리 관리 (핸들 → Queue 매핑 테이블 필요)
 */

#include "can_bsp.h"
#include <stdio.h>
#include <string.h>

/* ========================================================================== */
/*  내부 상태                                                                 */
/* ========================================================================== */

/* TODO: 실제 구현에서는 hcan 핸들별로 stats/queue 매핑 테이블을 둘 것 */
static CAN_Stats_t s_stats = {0};

/* ========================================================================== */
/*  초기화 / 필터                                                             */
/* ========================================================================== */

int CAN_BSP_Init(CAN_HandleTypeDef *hcan)
{
    (void)hcan;
    /* TODO: 실제 구현
     * HAL_CAN_Start(hcan);
     * HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
     */
    printf("[CAN_BSP] Init (stub)\r\n");
    memset(&s_stats, 0, sizeof(s_stats));
    return 0;
}

void CAN_BSP_SetRxQueue(CAN_HandleTypeDef *hcan, QueueHandle_t queue)
{
    (void)hcan;
    (void)queue;
    /* TODO: hcan → queue 매핑 테이블에 저장.
     *       RX 콜백에서 이 queue로 xQueueSendFromISR() */
    printf("[CAN_BSP] SetRxQueue (stub)\r\n");
}

int CAN_BSP_AddFilter(CAN_HandleTypeDef *hcan, uint32_t std_id)
{
    (void)hcan;
    /* TODO: CAN_FilterTypeDef 채워서 HAL_CAN_ConfigFilter() */
    printf("[CAN_BSP] AddFilter id=0x%03lX (stub)\r\n", (unsigned long)std_id);
    return 0;
}

int CAN_BSP_AddFilterAcceptAll(CAN_HandleTypeDef *hcan)
{
    (void)hcan;
    /* TODO: Mask mode, FilterId=0x0000, FilterMask=0x0000 */
    printf("[CAN_BSP] AddFilterAcceptAll (stub)\r\n");
    return 0;
}

/* ========================================================================== */
/*  송신                                                                      */
/* ========================================================================== */

int CAN_BSP_Send(CAN_HandleTypeDef *hcan, const CAN_Msg_t *msg)
{
    (void)hcan;
    if (msg == NULL || msg->dlc > 8) return -1;

    /* STUB: UART 로 TX 프레임 출력.
     * 실제 구현 시 HAL_CAN_AddTxMessage() 사용. */
    printf("[CAN TX] ID=0x%03lX DLC=%u Data=",
           (unsigned long)msg->id, (unsigned)msg->dlc);
    for (uint8_t i = 0; i < msg->dlc; i++) {
        printf("%02X ", msg->data[i]);
    }
    printf("\r\n");

    s_stats.tx_count++;
    return 0;
}

/* ========================================================================== */
/*  통계                                                                      */
/* ========================================================================== */

void CAN_BSP_GetStats(CAN_HandleTypeDef *hcan, CAN_Stats_t *out_stats)
{
    (void)hcan;
    if (out_stats) *out_stats = s_stats;
}

void CAN_BSP_ResetStats(CAN_HandleTypeDef *hcan)
{
    (void)hcan;
    memset(&s_stats, 0, sizeof(s_stats));
}
