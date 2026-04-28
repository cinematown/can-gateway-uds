#include "engine_sim.h"

#define ENG_PERIOD_RPM_MS        50
#define ENG_PERIOD_SPEED_MS      100
#define ENG_PERIOD_COOLANT_MS    1000
#define ENG_PERIOD_KEEPALIVE_MS  100

typedef struct
{
    EngineMode_t mode;

    uint8_t throttle;
    uint8_t brake;

    uint16_t rpm;
    uint16_t speed;
    uint8_t coolant;

    uint32_t tx_count;
} EngineSim_t;

static EngineSim_t engine = {
    .mode = ENGINE_MODE_ADC,
    .throttle = 0,
    .brake = 0,
    .rpm = 800,
    .speed = 0,
    .coolant = 70,
    .tx_count = 0
};

void EngineSim_Init(void)
{
    CAN_BSP_Init();
}

void EngineSim_SetMode(EngineMode_t mode)
{
    if (mode > ENGINE_MODE_RAW)
        return;

    engine.mode = mode;
}

void EngineSim_SetThrottle(uint8_t throttle)
{
    if (throttle > 100)
        throttle = 100;

    engine.throttle = throttle;
}

void EngineSim_SetBrake(uint8_t brake)
{
    if (brake > 100)
        brake = 100;

    engine.brake = brake;
}

void EngineSim_GetStatus(EngineSimStatus_t *status)
{
    if (status == NULL)
        return;

    status->mode = engine.mode;
    status->throttle = engine.throttle;
    status->brake = engine.brake;
    status->rpm = engine.rpm;
    status->speed = engine.speed;
    status->coolant = engine.coolant;
    status->tx_count = engine.tx_count;
}

const char *EngineSim_GetModeString(EngineMode_t mode)
{
    switch (mode)
    {
    case ENGINE_MODE_ADC:
        return "adc";
    case ENGINE_MODE_UART:
        return "uart";
    case ENGINE_MODE_RAW:
        return "raw";
    default:
        return "unknown";
    }
}

static void EngineSim_UpdateInput(void)
{
    if (engine.mode == ENGINE_MODE_ADC)
    {
        uint16_t throttle_adc = ADC_ReadThrottle();

        engine.throttle = (uint8_t)(((uint32_t)throttle_adc * 100) / 4095);

        // 현재 ADC 브레이크 입력이 없으면 0 고정
        engine.brake = 0;
    }
}

static void EngineSim_UpdatePhysics(void)
{
    uint16_t target_speed = engine.throttle;

    if (engine.brake > 0)
    {
        if (target_speed > engine.brake)
            target_speed -= engine.brake;
        else
            target_speed = 0;
    }

    if (engine.speed < target_speed)
        engine.speed++;
    else if (engine.speed > target_speed)
        engine.speed--;

    engine.rpm = 800 + (engine.speed * 40);

    engine.coolant = 70 + (engine.rpm / 100);
    if (engine.coolant > 120)
        engine.coolant = 120;
}

static void EngineSim_SendRpm(void)
{
    uint8_t data[8];

    SignalDb_EncodeMotor1Rpm(data, engine.rpm);

    if (CAN_BSP_Send(CAN_ID_ENGINE_RPM, data, 8) == HAL_OK)
        engine.tx_count++;
}

static void EngineSim_SendSpeed(void)
{
    uint8_t data[8];

    SignalDb_EncodeBremse1Speed(data, engine.speed);

    if (CAN_BSP_Send(CAN_ID_ENGINE_SPEED, data, 8) == HAL_OK)
        engine.tx_count++;
}

static void EngineSim_SendCoolant(void)
{
    uint8_t data[8];

    SignalDb_EncodeMotor2Coolant(data, engine.coolant);

    if (CAN_BSP_Send(CAN_ID_ENGINE_COOLANT, data, 8) == HAL_OK)
        engine.tx_count++;
}

static void EngineSim_SendKeepAlive(void)
{
    uint8_t data[8] = {0};

    data[0] = 0x01;  // alive flag
    data[1] = 0x00;

    if (CAN_BSP_Send(CAN_ID_ENGINE_KEEPALIVE, data, 8) == HAL_OK)
        engine.tx_count++;
}

void EngineSim_Task(void *argument)
{
    (void)argument;

    uint32_t t_rpm = 0;
    uint32_t t_speed = 0;
    uint32_t t_coolant = 0;
    uint32_t t_keepalive = 0;

    for (;;)
    {
        uint32_t now = osKernelGetTickCount();

        if (engine.mode != ENGINE_MODE_RAW)
        {
            EngineSim_UpdateInput();
            EngineSim_UpdatePhysics();

            if (now - t_rpm >= ENG_PERIOD_RPM_MS)
            {
                EngineSim_SendRpm();
                t_rpm = now;
            }

            if (now - t_speed >= ENG_PERIOD_SPEED_MS)
            {
                EngineSim_SendSpeed();
                t_speed = now;
            }

            if (now - t_coolant >= ENG_PERIOD_COOLANT_MS)
            {
                EngineSim_SendCoolant();
                t_coolant = now;
            }

            if (now - t_keepalive >= ENG_PERIOD_KEEPALIVE_MS)
            {
                EngineSim_SendKeepAlive();
                t_keepalive = now;
            }
        }

        osDelay(5);
    }
}


#include "uart.h"
#include "cli.h"
#include "cli_cmd.h"

void StartDefaultTask(void *argument)
{
    // 보드 A 초기화 세팅
    uartInit();
    cliInit();
    CliCmd_Init();
    EngineSim_Init();

    uartPrintf(0, "\033[2J\033[H");
    uartPrintf(0, "==================================\r\n");
    uartPrintf(0, "   [Board A] Engine ECU Simulator\r\n");
    uartPrintf(0, "==================================\r\n\r\n");

    for(;;)
    {
        cliMain();  // UART 명령어 처리
        osDelay(1);
    }
}
