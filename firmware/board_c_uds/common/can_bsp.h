#ifndef CAN_BSP_H
#define CAN_BSP_H

#include "main.h"
#include "cmsis_os2.h"
#include "protocol_ids.h"
#include <stdint.h>
#include <stdbool.h>

/*
 * Board C uses CAN1.
 * bus 값은 수신 메시지가 어느 CAN 채널에서 들어왔는지 구분하기 위한 값입니다.
 */
#define CAN_BSP_BUS_CAN1    1u
#define CAN_BSP_BUS_CAN2    2u
#define CAN_BSP_BOARD_BUS   CAN_BSP_BUS_CAN1

typedef struct
{
    uint8_t  bus;       /* 1: CAN1, 2: CAN2 */
    uint32_t id;        /* Standard CAN ID */
    uint8_t  dlc;       /* Data Length Code */
    uint8_t  data[8];   /* CAN payload */
} CAN_RxMessage_t;

extern osMessageQueueId_t can_rx_q;

/* CLI/debug용 통계 카운터 */
extern volatile uint32_t can1TxCount;
extern volatile uint32_t can1RxCount;
extern volatile uint32_t canSendEnterCount;
extern volatile uint32_t canTxBusyCount;
extern volatile uint32_t canTxErrorCount;

HAL_StatusTypeDef CAN_BSP_Init(void);
HAL_StatusTypeDef CAN_BSP_Send(uint32_t id, uint8_t *data, uint8_t len);
HAL_StatusTypeDef CAN_BSP_SendTo(CAN_HandleTypeDef *hcan, uint32_t id, uint8_t *data, uint8_t len);
bool CAN_BSP_Read(CAN_RxMessage_t *p_msg, uint32_t timeout);

#endif /* CAN_BSP_H */
