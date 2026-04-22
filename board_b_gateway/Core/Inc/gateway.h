/**
 * @file    gateway.h
 * @brief   Central Gateway - CAN1 → CAN2 라우팅 + 이상 감지 + 트래픽 로거
 * @author  ③ 지윤
 * @board   보드B (STM32 Nucleo-F446RE 필수 - CAN1/CAN2 2포트)
 *
 *   역할:
 *     - CAN1 RX Queue 에서 메시지 pop
 *     - 라우팅 테이블 조회 → CAN2 TX
 *     - RPM > RPM_THRESHOLD_WARNING 시 경고 메시지 CAN2 송신
 *     - 초당 TX/RX/Error 카운터 UART 출력 (트래픽 로거)
 *
 *   TODO(③):
 *     [ ] 라우팅 테이블 자료구조 + AddRoute API
 *     [ ] 초기 라우팅 엔트리 등록 (RPM/Speed/Coolant pass-through)
 *     [ ] 이상 감지 로직 (hysteresis 포함 권장)
 *     [ ] 트래픽 통계 Task (1초 주기)
 */

#ifndef GATEWAY_H
#define GATEWAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GATEWAY_ROUTE_TABLE_MAX   16U

/**
 * @brief 라우팅 엔트리: src_id → dst_id.
 * @note  src_id == dst_id 도 허용 (pass-through).
 */
typedef struct {
    uint32_t src_id;
    uint32_t dst_id;
    uint8_t  enabled;
} Gateway_Route_t;

/**
 * @brief Gateway 초기화.
 */
void Gateway_Init(void);

/**
 * @brief 라우팅 테이블에 엔트리 추가.
 * @retval 0: OK, -1: 테이블 가득 참
 */
int Gateway_AddRoute(uint32_t src_id, uint32_t dst_id);

/**
 * @brief FreeRTOS Task - CAN1 RX Queue pending 후 CAN2 포워딩.
 */
void Gateway_Task(void *argument);

/**
 * @brief 트래픽 로거 Task - 1초 주기 UART 출력.
 */
void Gateway_LoggerTask(void *argument);

/* ========== 이상 감지 상태 조회 ========== */
uint8_t Gateway_IsWarningActive(void);

#ifdef __cplusplus
}
#endif

#endif /* GATEWAY_H */
