/**
 * @file    can_bsp.h
 * @brief   CAN Board Support Package - CAN1/CAN2 공통 드라이버 인터페이스
 * @author  ① 성재, 민진
 * @note    - 이 헤더는 팀장 관리 대상. 시그니처 변경 시 전원 공지.
 *          - 실제 구현은 can_bsp.c에 작성.
 *          - Day 2에 stub 버전 선배포 → 나머지 팀원이 독립 개발 가능.
 */

#ifndef COMMON_CAN_BSP_H
#define COMMON_CAN_BSP_H

#include <stdint.h>
#include "can_db.h"

/* CubeMX 생성 HAL/FreeRTOS 헤더는 각 보드 프로젝트의 can_bsp.c에서 include */
#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration — HAL 의존을 헤더에서 제거 */
struct __CAN_HandleTypeDef;
typedef struct __CAN_HandleTypeDef CAN_HandleTypeDef;
struct QueueDefinition;
typedef struct QueueDefinition * QueueHandle_t;

/* ========================================================================== */
/*  초기화 / 설정                                                             */
/* ========================================================================== */

/**
 * @brief CAN 페리페럴 초기화 (500kbps, Normal Mode).
 * @param hcan  CubeMX에서 생성한 CAN 핸들
 * @retval 0: OK, -1: 실패
 * @note  이 함수 호출 전 HAL_CAN_Init()은 CubeMX init이 수행한다.
 *        이 함수는 필터 기본 설정 + Notification 활성화만 담당.
 */
int CAN_BSP_Init(CAN_HandleTypeDef *hcan);

/**
 * @brief RX 메시지를 수신할 FreeRTOS Queue 등록.
 * @param hcan   대상 CAN 핸들
 * @param queue  CAN_Msg_t 형식을 담는 Queue (용량 16 권장)
 * @note  RX 콜백 내부에서 xQueueSendFromISR()로 push 한다.
 */
void CAN_BSP_SetRxQueue(CAN_HandleTypeDef *hcan, QueueHandle_t queue);

/**
 * @brief 수신할 CAN ID 필터 추가 (여러 번 호출 가능).
 * @param hcan     대상 CAN 핸들
 * @param std_id   11-bit standard ID
 * @retval 0: OK, -1: 필터 슬롯 초과
 */
int CAN_BSP_AddFilter(CAN_HandleTypeDef *hcan, uint32_t std_id);

/**
 * @brief 모든 ID를 수신하는 Accept-All 필터 설정 (게이트웨이/로거용).
 */
int CAN_BSP_AddFilterAcceptAll(CAN_HandleTypeDef *hcan);

/* ========================================================================== */
/*  송신                                                                      */
/* ========================================================================== */

/**
 * @brief CAN 메시지 송신 (polling, non-blocking).
 * @param hcan 대상 CAN 핸들
 * @param msg  송신 메시지 (id, data, dlc)
 * @retval 0: OK (mailbox 등록 성공), -1: 3개 mailbox 모두 사용 중
 */
int CAN_BSP_Send(CAN_HandleTypeDef *hcan, const CAN_Msg_t *msg);

/* ========================================================================== */
/*  통계 (로거용)                                                             */
/* ========================================================================== */

typedef struct {
    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t error_count;
    uint32_t overrun_count;
} CAN_Stats_t;

void CAN_BSP_GetStats(CAN_HandleTypeDef *hcan, CAN_Stats_t *out_stats);
void CAN_BSP_ResetStats(CAN_HandleTypeDef *hcan);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_CAN_BSP_H */
