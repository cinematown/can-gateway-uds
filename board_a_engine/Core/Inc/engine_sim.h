/**
 * @file    engine_sim.h
 * @brief   Engine ECU Simulator - 가변저항(페달) → CAN1 신호 생성
 * @author  ② 윤지
 * @board   보드A (STM32 Nucleo-F446RE or F103RB)
 *
 *   역할:
 *     - 가변저항 2개(액셀/브레이크) ADC 값 읽기
 *     - RPM, Speed, Coolant 신호를 주기적으로 CAN1에 송신
 *     - 의존: common/can_bsp/ + common/can_db/
 *
 *   TODO(②):
 *     [ ] ADC DMA 설정 (ADC1_IN0, ADC1_IN1)
 *     [ ] ADC → RPM 매핑 함수 (0~4095 → 800~7000 rpm)
 *     [ ] ADC → Speed 매핑 함수 (0~4095 → 0~260 km/h)
 *     [ ] FreeRTOS Task 생성 (송신 주기 관리)
 *     [ ] keep-alive: 계기판이 요구하는 필수 ID 주기 송신
 */

#ifndef ENGINE_SIM_H
#define ENGINE_SIM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 엔진 시뮬레이터 초기화 (ADC, 내부 상태).
 */
void EngSim_Init(void);

/**
 * @brief FreeRTOS Task 엔트리. 주기 송신 루프.
 * @note  50ms/100ms/1000ms 주기를 vTaskDelayUntil 로 관리.
 */
void EngSim_Task(void *argument);

/* ========== 외부 조회 API (디버그/로그용) ========== */
uint16_t EngSim_GetRPM(void);
uint16_t EngSim_GetSpeed(void);    /* km/h */
int8_t   EngSim_GetCoolantTemp(void);

#ifdef __cplusplus
}
#endif

#endif /* ENGINE_SIM_H */
