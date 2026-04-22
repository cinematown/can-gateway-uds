/**
 * @file    main.c
 * @brief   보드A (Engine ECU Simulator) 엔트리
 * @author  ⑥ 한결 (통합) + ② 윤지 (모듈)
 *
 *   ⚠️ 이 파일은 CubeMX 가 생성/관리합니다.
 *      실제 프로젝트 생성 시 CubeMX 가 만든 main.c 로 교체되며,
 *      이 파일은 "어디에 무엇을 추가해야 하는지" 를 보여주는 가이드입니다.
 *
 *   추가해야 할 것:
 *     1. #include "engine_sim.h"
 *     2. #include "can_bsp.h"
 *     3. Queue 생성: can1_rx_queue
 *     4. CAN_BSP_Init(&hcan1);
 *     5. CAN_BSP_SetRxQueue(&hcan1, can1_rx_queue);  (이 보드는 RX 불필요 시 생략)
 *     6. EngSim_Init();
 *     7. osThreadNew(EngSim_Task, ...);
 *     8. osKernelStart();
 */

/* USER CODE BEGIN Includes */
#include "engine_sim.h"
#include "can_bsp.h"
#include "can_db.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* 이 보드는 CAN TX 전용이라 RX Queue는 선택 */
/* QueueHandle_t can1_rx_queue; */
/* USER CODE END PV */

int main(void)
{
    /* CubeMX generated: HAL_Init, SystemClock_Config, MX_*_Init */

    /* USER CODE BEGIN 2 */
    CAN_BSP_Init(&hcan1);
    EngSim_Init();
    /* USER CODE END 2 */

    /* CubeMX: MX_FREERTOS_Init() — 여기서 osThreadNew(EngSim_Task, ...) 추가 */

    /* osKernelStart(); */

    while (1) { }
}
