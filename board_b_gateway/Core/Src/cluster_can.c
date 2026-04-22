/**
 * @file    cluster_can.c
 * @brief   VW Passat B6 계기판 구현 (STUB)
 * @author  ⑤ 미정
 */

#include "cluster_can.h"
#include "can_bsp.h"
#include "can_db.h"
#include <string.h>

/* TODO: extern CAN_HandleTypeDef hcan2; */

static uint16_t s_rpm = 800U;
static uint16_t s_speed = 0U;
static int8_t   s_temp = 25;
static uint8_t  s_warn = 0U;

void Cluster_Init(void)
{
    s_rpm = 800U;
    s_speed = 0U;
    s_temp = 25;
    s_warn = 0U;
}

void Cluster_SendInitSequence(void)
{
    /* TODO: VW 계기판 초기화 시퀀스
     *   - 에어백 경고등 off (0x050, byte[1]=0x80)
     *   - ABS 경고등 off (0x1A0, byte[0]=0x18)
     *   - 바늘 풀스윙 효과 (RPM을 0→MAX→0 으로 1초간 램프) */
}

void Cluster_SetRPM(uint16_t rpm)              { s_rpm = rpm; }
void Cluster_SetSpeed(uint16_t kmh)            { s_speed = kmh; }
void Cluster_SetCoolantTemp(int8_t celsius)    { s_temp = celsius; }
void Cluster_SetWarning(uint8_t on)            { s_warn = on ? 1U : 0U; }

static void tx_warning(void)
{
    CAN_Msg_t m = { .id = CAN_ID_WARNING, .dlc = 8 };
    memset(m.data, 0, 8);
    if (s_warn) m.data[0] = WARN_BIT_RPM_OVER;
    /* TODO: CAN_BSP_Send(&hcan2, &m); */
    (void)m;
}

void Cluster_Task(void *argument)
{
    (void)argument;

    Cluster_SendInitSequence();

    uint32_t tick = 0;
    for (;;) {
        /* 계기판은 Gateway가 포워딩하는 메시지를 이미 받고 있음.
         * 이 Task는 "경고등/온도/keep-alive" 처럼 Gateway가 생성하지 않는
         * 메시지만 담당. */
        if (tick % 100U == 0) tx_warning();

        /* TODO: osDelay(10); */
        tick += 10;
        break; /* STUB */
    }
}
