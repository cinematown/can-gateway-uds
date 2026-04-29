#ifndef CAN_BSP_H
#define CAN_BSP_H

#include "hw_def.h"
#include "cli.h"

/* --- 타입 정의 --- */

/**
 * @brief CAN 수신 메시지를 담기 위한 구조체
 */
typedef struct {
    uint32_t id;           // CAN ID (표준 11비트 또는 확장 29비트)
    uint8_t  dlc;          // 데이터 길이 (Data Length Code)
    uint8_t  data[8];      // 실제 데이터 8바이트
    CAN_RxHeaderTypeDef header; // HAL 계층의 헤더 정보 보관용
} CAN_RxMessage_t;

/* --- 외부 핸들러 참조 --- */
// main.c에서 CubeMX가 생성한 핸들러를 가져옵니다.
extern CAN_HandleTypeDef hcan1; 
// 만약 Board C에서 두 개의 CAN을 다 쓴다면 hcan2도 추가하세요.
// extern CAN_HandleTypeDef hcan2; 

/* --- 함수 선언 --- */

/**
 * @brief CAN 하드웨어 초기화 및 필터 설정
 */
void CAN_BSP_Init(void);

/**
 * @brief 특정 ID만 통과시키는 필터 설정
 * @param id 통과시킬 CAN ID (예: 0x7E8)
 */
void CAN_BSP_ConfigFilter_Open(void);
void CAN_BSP_ConfigFilter(uint32_t id);
void CAN_BSP_ConfigFilter_UDS_Response(void);
void UDS_Request_ReadData(uint32_t canId, uint16_t did);
/**
 * @brief 수신 FIFO에서 메시지 한 개 가져오기
 * @return HAL_OK: 성공, HAL_ERROR: 메시지 없음
 */
HAL_StatusTypeDef CAN_BSP_GetRxMessage(CAN_RxMessage_t *pMsg);


#endif /* CAN_BSP_H */