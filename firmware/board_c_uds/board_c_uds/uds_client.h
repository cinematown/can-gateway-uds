#ifndef UDS_CLIENT_H
#define UDS_CLIENT_H

#include <stdint.h>
#include <stdbool.h>
#include "can_bsp.h"
#include "protocol_ids.h"
#include "uds_defs.h"

typedef struct
{
    uint32_t req_id;
    uint32_t resp_id;
    uint16_t last_did;
    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t err_count;
    bool     log_on;
} UDS_ClientStatus_t;

void UDS_Client_Init(void);
void UDS_Client_SetTarget(uint32_t req_id, uint32_t resp_id);
void UDS_Client_GetStatus(UDS_ClientStatus_t *out);
void UDS_Client_SetCanLog(bool on);
bool UDS_Client_GetCanLog(void);

bool UDS_Client_ReadDID(uint16_t did);

/*
 * Team-style blocking diagnostic loop.
 * - 3초마다 UDS ReadDataByIdentifier 요청 재전송
 * - CAN1로 들어오는 프레임을 모두 출력
 * - 계기판 응답 ID(기본 0x77E)가 들어오면 HIT 표시
 * - 아무 UART 키나 누르면 종료
 */
void UDS_Client_ExecuteDiagnostic(uint16_t did, const char *label);

/* 팀원 코드와 비슷한 함수명으로도 호출 가능하게 만든 wrapper */
void UDS_Execute_Diagnostic(uint16_t did, const char *label);
void UDS_Request_ReadData(uint32_t canId, uint16_t did);

void UDS_Client_OnCanRx(const CAN_RxMessage_t *msg);

#endif /* UDS_CLIENT_H */
