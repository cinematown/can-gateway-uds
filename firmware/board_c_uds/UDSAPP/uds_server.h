#ifndef UDS_SERVER_H
#define UDS_SERVER_H

#include "hw_def.h"
#include "main.h"
#include "protocol_ids.h"
/* UDS 서비스 ID 정의 */
#define SID_READ_DATA_BY_ID         0x22
#define SID_READ_DATA_BY_ID_POS_RES 0x62

/* DID (Data Identifier) 정의 */
#define DID_VEHICLE_SPEED           0x0D00
#define DID_ENGINE_RPM              0x0C00

/* 함수 선언 */
void UDS_Server_Init(void);
void UDS_Server_Process(void); // CAN 수신 메시지 확인 및 값 변환 출력

#endif /* UDS_SERVER_H */