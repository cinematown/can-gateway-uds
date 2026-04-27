#include "engine_sim.h"



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
    CAN_Driver_Init();
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
        uint16_t brake_adc = 0;

        engine.throttle = (uint8_t)(((uint32_t)throttle_adc * 100) / 4095);
        engine.brake    = (uint8_t)(((uint32_t)brake_adc * 100) / 4095);
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
    {
        engine.speed++;
    }
    else if (engine.speed > target_speed)
    {
        engine.speed--;
    }

    engine.rpm = 800 + (engine.speed * 40);
    engine.coolant = 70 + (engine.rpm / 100);

    if (engine.coolant > 120)
        engine.coolant = 120;
}

static void EngineSim_SendCan(void)
{
    uint8_t data[ENGINE_CAN_DLC] = {0};

    CAN_PutU16LE(data, 0, engine.rpm);
    CAN_PutU16LE(data, 2, engine.speed);

    data[4] = engine.coolant;
    data[5] = engine.throttle;
    data[6] = engine.brake;
    data[7] = 0;

    if (CAN1_Send(ENGINE_CAN_ID, data, ENGINE_CAN_DLC) == HAL_OK)
    {
        engine.tx_count++;
    }
}

void EngineSim_Task(void *argument)
{
    (void)argument;

    for (;;)
    {
        if (engine.mode != ENGINE_MODE_RAW)
        {
            EngineSim_UpdateInput();
            EngineSim_UpdatePhysics();
            EngineSim_SendCan();
        }

        osDelay(100);
    }
}