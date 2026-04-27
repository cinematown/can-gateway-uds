#ifndef ENGINE_CAN_H
#define ENGINE_CAN_H

#include <stdint.h>

#define ENGINE_CAN_ID        0x100
#define ENGINE_CAN_DLC       8

static inline void CAN_PutU16LE(uint8_t *buf, uint8_t index, uint16_t value)
{
    buf[index]     = value & 0xFF;
    buf[index + 1] = (value >> 8) & 0xFF;
}

static inline uint16_t CAN_GetU16LE(uint8_t *buf, uint8_t index)
{
    return ((uint16_t)buf[index + 1] << 8) | buf[index];
}

#endif

// freertos 백업용
// #include "engine_sim.h"
// #include "ap.h"


// /* USER CODE BEGIN Header_StartDefaultTask */
// /**
//   * @brief  Function implementing the defaultTask thread.
//   * @param  argument: Not used
//   * @retval None
//   */
// /* USER CODE END Header_StartDefaultTask */
// void StartDefaultTask(void *argument)
// {
//   apInit();  // RTOS 스케줄러가 구동된 후, 여기서 모든 앱/드라이버 초기화
//   apMain();  // 무한 루프 진입 (CLI 처리 등)
//   /* USER CODE END StartDefaultTask */
// }

// /* USER CODE BEGIN Header_StartEngSimTask */
// /**
// * @brief Function implementing the EngSimTask thread.
// * @param argument: Not used
// * @retval None
// */
// /* USER CODE END Header_StartEngSimTask */
// void StartEngSimTask(void *argument)
// {
//   /* USER CODE BEGIN StartEngSimTask */
//   EngineSim_Task(argument); // 엔진 시뮬레이션 로직 실행
//   /* USER CODE END StartEngSimTask */
// }
