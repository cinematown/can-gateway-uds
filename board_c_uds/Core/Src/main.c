/**
 * @file    main.c
 * @brief   보드C (UDS Diagnostic Server) 엔트리
 * @author  ⑥ 한결 (통합) + ④ 은빈 (모듈)
 *
 *   ⚠️ 이 파일은 CubeMX 가 관리. 여기는 가이드.
 *
 *   추가할 것:
 *     1. Queue 생성: can_rx_queue
 *     2. CAN_BSP_Init(&hcan1);  // F103RB면 CAN, F446RE면 CAN1 or CAN2
 *     3. CAN_BSP_SetRxQueue(&hcan1, can_rx_queue);
 *     4. UDS 요청만 수신하려면 AddFilter(CAN_ID_UDS_REQ),
 *        캐시를 위해 RPM/Speed/Temp도 수신하려면 AcceptAll 권장
 *     5. UDS_Init();
 *     6. osThreadNew(UDS_Task, ...);
 *     7. osThreadNew(UDS_CliTask, ...);
 *     8. osKernelStart();
 */

/* USER CODE BEGIN Includes */
#include "uds_server.h"
#include "can_bsp.h"
#include "can_db.h"
/* USER CODE END Includes */

int main(void)
{
    /* CubeMX generated init */

    /* USER CODE BEGIN 2 */
    CAN_BSP_Init(&hcan1);
    CAN_BSP_AddFilterAcceptAll(&hcan1);
    UDS_Init();
    /* USER CODE END 2 */

    /* Task 생성 + osKernelStart */

    while (1) { }
}
