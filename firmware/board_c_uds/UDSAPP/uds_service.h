#ifndef UDS_SERVICE_H
#define UDS_SERVICE_H

#include "main.h"      // HAL 및 기본 자료형(uint16_t 등) 사용을 위해 포함
#include "can_bsp.h"   // CAN_RxMessage_t 자료형 사용을 위해 포함

/**
 * @brief  UDS 통합 진단 실행 함수
 * @param  did   : 요청할 Data Identifier (예: UDS_DID_SPEED)
 * @param  label : CLI 출력용 라벨 문자열 (예: "SPEED")
 * @details 이 함수는 protocol_ids.h에 정의된 REQ_ID로 요청을 보내고, 
 * RESP_ID로 들어오는 응답을 대기한 뒤 결과를 CLI에 출력합니다.
 */
void UDS_Execute_Diagnostic(uint16_t did, const char* label);

/**
 * @brief  UDS 데이터 전송 함수 (추상화된 전송 레이어)
 * @param  canId : 송신할 CAN ID
 * @param  did   : 요청할 DID
 */
void UDS_Request_ReadData(uint32_t canId, uint16_t did);

#endif /* UDS_SERVICE_H */