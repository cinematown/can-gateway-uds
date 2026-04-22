/**
 * @file    engine_sim.c
 * @brief   Engine ECU Simulator 구현 (STUB)
 * @author  ② 윤지
 *
 *   이 파일은 STUB 입니다. 아래 TODO를 채워주세요.
 */

#include "engine_sim.h"
#include "can_bsp.h"
#include "can_db.h"
#include <string.h>

/* TODO: FreeRTOS 헤더 include */
/* #include "cmsis_os.h" */

/* TODO: CubeMX 핸들 extern */
/* extern CAN_HandleTypeDef hcan1; */
/* extern ADC_HandleTypeDef hadc1; */

/* ========================================================================== */
/*  내부 상태                                                                 */
/* ========================================================================== */

static uint16_t s_rpm      = 800U;    /* Idle */
static uint16_t s_speed    = 0U;
static int8_t   s_coolant  = 25;      /* 25°C */

/* ========================================================================== */
/*  초기화                                                                    */
/* ========================================================================== */

void EngSim_Init(void)
{
    /* TODO:
     *   1. HAL_ADC_Start_DMA() 또는 polling 준비
     *   2. 초기값 세팅
     */
    s_rpm = 800U;
    s_speed = 0U;
    s_coolant = 25;
}

/* ========================================================================== */
/*  ADC → 신호 매핑 (내부 함수)                                               */
/* ========================================================================== */

static uint16_t map_adc_to_rpm(uint16_t adc_val)
{
    /* TODO: 0~4095 → 800~7000 rpm 선형 매핑
     *   return 800 + (uint16_t)((uint32_t)adc_val * (7000-800) / 4095);
     */
    return 800U + (uint16_t)((uint32_t)adc_val * 6200U / 4095U);
}

static uint16_t map_adc_to_speed(uint16_t adc_val)
{
    /* TODO: 0~4095 → 0~260 km/h */
    return (uint16_t)((uint32_t)adc_val * 260U / 4095U);
}

/* ========================================================================== */
/*  CAN 송신 (내부 함수)                                                      */
/* ========================================================================== */

static void send_rpm(uint16_t rpm)
{
    CAN_Msg_t msg = { .id = CAN_ID_RPM, .dlc = 8 };
    memset(msg.data, 0, 8);
    /* VW Passat B6: byte[2:3] = rpm × 4 (little-endian) */
    uint16_t raw = ENCODE_RPM(rpm);
    msg.data[2] = (uint8_t)(raw & 0xFF);
    msg.data[3] = (uint8_t)(raw >> 8);
    /* TODO: CAN_BSP_Send(&hcan1, &msg); */
    (void)msg;
}

static void send_speed(uint16_t kmh)
{
    CAN_Msg_t msg = { .id = CAN_ID_SPEED, .dlc = 8 };
    memset(msg.data, 0, 8);
    /* byte[0] = 0x18 (ABS 경고등 끄기 keep-alive) */
    msg.data[0] = 0x18;
    uint16_t raw = ENCODE_SPEED(kmh);
    msg.data[2] = (uint8_t)(raw & 0xFF);
    msg.data[3] = (uint8_t)(raw >> 8);
    /* TODO: CAN_BSP_Send(&hcan1, &msg); */
    (void)msg;
}

static void send_coolant(int8_t celsius)
{
    CAN_Msg_t msg = { .id = CAN_ID_COOLANT, .dlc = 8 };
    memset(msg.data, 0, 8);
    msg.data[3] = ENCODE_TEMP(celsius);
    /* TODO: CAN_BSP_Send(&hcan1, &msg); */
    (void)msg;
}

/* ========================================================================== */
/*  Task                                                                      */
/* ========================================================================== */

void EngSim_Task(void *argument)
{
    (void)argument;
    uint32_t tick = 0;

    for (;;) {
        /* TODO: ADC 값 읽어와서 s_rpm / s_speed 업데이트
         *   uint16_t adc_accel = read_adc_channel(0);
         *   uint16_t adc_brake = read_adc_channel(1);
         *   s_rpm   = map_adc_to_rpm(adc_accel);
         *   s_speed = map_adc_to_speed(adc_accel);
         *   if (adc_brake > 2048) s_speed /= 2;
         */

        /* 주기별 송신 */
        if (tick % PERIOD_RPM_MS == 0)      send_rpm(s_rpm);
        if (tick % PERIOD_SPEED_MS == 0)    send_speed(s_speed);
        if (tick % PERIOD_COOLANT_MS == 0)  send_coolant(s_coolant);

        /* TODO: osDelay(10) 또는 vTaskDelayUntil 로 10ms 주기 유지 */
        tick += 10;
    }
}

/* ========================================================================== */
/*  조회 API                                                                  */
/* ========================================================================== */

uint16_t EngSim_GetRPM(void)         { return s_rpm; }
uint16_t EngSim_GetSpeed(void)       { return s_speed; }
int8_t   EngSim_GetCoolantTemp(void) { return s_coolant; }
