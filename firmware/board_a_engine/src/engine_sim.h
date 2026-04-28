#ifndef ENGINE_SIM_H
#define ENGINE_SIM_H

#include "main.h"
#include <stdint.h>
#include "cmsis_os2.h"
#include "adc_driver.h"
#include "can_bsp.h"
#include "engine_can.h"

typedef enum
{
    ENGINE_MODE_ADC = 0,
    ENGINE_MODE_UART,
    ENGINE_MODE_RAW
} EngineMode_t;

typedef struct
{
    EngineMode_t mode;

    uint8_t throttle;
    uint8_t brake;

    uint16_t rpm;
    uint16_t speed;
    uint8_t coolant;

    uint32_t tx_count;
} EngineSimStatus_t;

void EngineSim_Init(void);
void EngineSim_Task(void* argument);

void EngineSim_SetMode(EngineMode_t mode);
void EngineSim_SetThrottle(uint8_t throttle);
void EngineSim_SetBrake(uint8_t brake);

void EngineSim_GetStatus(EngineSimStatus_t* status);

const char* EngineSim_GetModeString(EngineMode_t mode);

#endif