/**
 * @file    main.c
 * @brief   보드B (Central Gateway) 엔트리
 * @author  ⑥ 한결 (통합) + ③ 지윤 + ⑤ 미정 (모듈)
 *
 *   ⚠️ 이 파일은 CubeMX 가 관리. 여기는 가이드.
 *
 *   추가할 것:
 *     1. Queue 생성: can1_rx_queue (16 items), can2_rx_queue (16 items, 선택)
 *     2. CAN_BSP_Init(&hcan1); CAN_BSP_Init(&hcan2);
 *     3. CAN_BSP_SetRxQueue(&hcan1, can1_rx_queue);
 *     4. CAN_BSP_AddFilterAcceptAll(&hcan1);  // Gateway는 전 ID 수신
 *     5. Gateway_Init(); Cluster_Init();
 *     6. osThreadNew(Gateway_Task, ...);
 *     7. osThreadNew(Gateway_LoggerTask, ...);
 *     8. osThreadNew(Cluster_Task, ...);
 *     9. osKernelStart();
 */

/* USER CODE BEGIN Includes */
#include "gateway.h"
#include "cluster_can.h"
#include "can_bsp.h"
#include "can_db.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* QueueHandle_t can1_rx_queue; */
/* QueueHandle_t can2_rx_queue; */
/* USER CODE END PV */

int main(void)
{
    /* CubeMX generated init */

    /* USER CODE BEGIN 2 */
    CAN_BSP_Init(&hcan1);
    CAN_BSP_Init(&hcan2);

    /* Gateway는 전 ID 수신 */
    CAN_BSP_AddFilterAcceptAll(&hcan1);

    /* TODO: Queue 생성 후 SetRxQueue 등록 */
    /* can1_rx_queue = xQueueCreate(16, sizeof(CAN_Msg_t)); */
    /* CAN_BSP_SetRxQueue(&hcan1, can1_rx_queue); */

    Gateway_Init();
    Cluster_Init();
    /* USER CODE END 2 */

    /* osThreadNew 들로 Task 생성 */

    while (1) { }
}
