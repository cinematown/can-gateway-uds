/**
 * @file    gateway.c
 * @brief   Central Gateway 구현 (STUB)
 * @author  ③ 지윤
 */

#include "gateway.h"
#include "can_bsp.h"
#include "can_db.h"
#include <string.h>
#include <stdio.h>

/* TODO: #include "cmsis_os.h" */
/* TODO: extern CAN_HandleTypeDef hcan1, hcan2; */
/* TODO: extern QueueHandle_t can1_rx_queue;   // main.c에서 생성 */

/* ========================================================================== */
/*  라우팅 테이블                                                             */
/* ========================================================================== */

static Gateway_Route_t s_routes[GATEWAY_ROUTE_TABLE_MAX];
static uint8_t s_route_count = 0U;

static uint8_t  s_warning_active = 0U;
static uint32_t s_rx_total = 0, s_tx_total = 0, s_drop_total = 0;

/* ========================================================================== */

void Gateway_Init(void)
{
    memset(s_routes, 0, sizeof(s_routes));
    s_route_count = 0U;
    s_warning_active = 0U;

    /* 기본 라우팅 등록 (CAN1 Powertrain → CAN2 Diagnostic/Cluster) */
    Gateway_AddRoute(CAN_ID_RPM,     CAN_ID_RPM);
    Gateway_AddRoute(CAN_ID_SPEED,   CAN_ID_SPEED);
    Gateway_AddRoute(CAN_ID_COOLANT, CAN_ID_COOLANT);
}

int Gateway_AddRoute(uint32_t src_id, uint32_t dst_id)
{
    if (s_route_count >= GATEWAY_ROUTE_TABLE_MAX) return -1;
    s_routes[s_route_count].src_id  = src_id;
    s_routes[s_route_count].dst_id  = dst_id;
    s_routes[s_route_count].enabled = 1U;
    s_route_count++;
    return 0;
}

static int find_route(uint32_t src_id, uint32_t *out_dst)
{
    for (uint8_t i = 0; i < s_route_count; i++) {
        if (s_routes[i].enabled && s_routes[i].src_id == src_id) {
            *out_dst = s_routes[i].dst_id;
            return 0;
        }
    }
    return -1;
}

/* ========================================================================== */
/*  이상 감지                                                                 */
/* ========================================================================== */

static void check_anomaly(const CAN_Msg_t *msg)
{
    if (msg->id == CAN_ID_RPM) {
        uint16_t raw = (uint16_t)msg->data[2] | ((uint16_t)msg->data[3] << 8);
        uint16_t rpm = DECODE_RPM(raw);

        if (rpm > RPM_THRESHOLD_WARNING && !s_warning_active) {
            s_warning_active = 1U;
            /* 경고 메시지 CAN2 송신 */
            CAN_Msg_t warn = { .id = CAN_ID_WARNING, .dlc = 8 };
            memset(warn.data, 0, 8);
            warn.data[0] = WARN_BIT_RPM_OVER;
            /* TODO: CAN_BSP_Send(&hcan2, &warn); */
            (void)warn;
            printf("[GW] WARNING: RPM over threshold (%u)\r\n", rpm);
        } else if (rpm < (RPM_THRESHOLD_WARNING - 500U) && s_warning_active) {
            /* hysteresis: 500 rpm 여유 후 해제 */
            s_warning_active = 0U;
            CAN_Msg_t warn = { .id = CAN_ID_WARNING, .dlc = 8 };
            memset(warn.data, 0, 8);
            /* TODO: CAN_BSP_Send(&hcan2, &warn); */
            (void)warn;
        }
    }
}

/* ========================================================================== */
/*  포워딩 Task                                                               */
/* ========================================================================== */

void Gateway_Task(void *argument)
{
    (void)argument;
    CAN_Msg_t rx;

    for (;;) {
        /* TODO: xQueueReceive(can1_rx_queue, &rx, portMAX_DELAY);
         *       — RX Queue 에 메시지가 올 때까지 대기 */

        /* STUB: 실제로는 ISR 콜백이 Queue push 해주면 여기가 깨어남 */
        memset(&rx, 0, sizeof(rx));

        s_rx_total++;

        uint32_t dst_id;
        if (find_route(rx.id, &dst_id) == 0) {
            /* 이상 감지 */
            check_anomaly(&rx);

            /* 포워딩 */
            CAN_Msg_t tx = rx;
            tx.id = dst_id;
            /* TODO: CAN_BSP_Send(&hcan2, &tx); */
            s_tx_total++;
        } else {
            s_drop_total++;
            /* Unknown ID — 로그만 (선택) */
        }

        /* STUB에서는 루프 폭주 방지용 */
        break;
    }
}

/* ========================================================================== */
/*  로거 Task                                                                 */
/* ========================================================================== */

void Gateway_LoggerTask(void *argument)
{
    (void)argument;
    static uint32_t prev_rx = 0, prev_tx = 0;

    for (;;) {
        uint32_t rx_rate = s_rx_total - prev_rx;
        uint32_t tx_rate = s_tx_total - prev_tx;
        prev_rx = s_rx_total;
        prev_tx = s_tx_total;

        printf("[GW-LOG] RX=%lu/s TX=%lu/s DROP_TOTAL=%lu WARN=%u\r\n",
               (unsigned long)rx_rate,
               (unsigned long)tx_rate,
               (unsigned long)s_drop_total,
               (unsigned)s_warning_active);

        /* TODO: osDelay(1000); */
        break;
    }
}

/* ========================================================================== */

uint8_t Gateway_IsWarningActive(void) { return s_warning_active; }
