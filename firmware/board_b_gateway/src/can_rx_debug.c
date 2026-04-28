/* --- START OF FILE board_b_gateway/can_rx_debug.c --- */
#include "can_rx_debug.h"
#include "can_bsp.h"    // 공통 CAN 드라이버
#include "signal_db.h"
#include "cmsis_os2.h"

volatile uint32_t canRxCount = 0;
volatile uint16_t rx_rpm = 0;
volatile uint16_t rx_speed = 0;
volatile uint8_t rx_coolant = 0;
volatile uint8_t rx_throttle = 0;
volatile uint8_t rx_brake = 0;

void CAN_RxDebug_Task(void *argument)
{
    // 공통 CAN 드라이버 초기화 (필터 설정, 큐 생성, 인터럽트 ON)
    CAN_BSP_Init();

    CAN_RxMessage_t rxMsg;

    for(;;)
    {
        // 큐에 데이터가 들어올 때까지 무한 대기 (CPU 0%)
        if (CAN_BSP_Read(&rxMsg, osWaitForever))
        {
            // 받은 편지가 엔진(0x100)에서 온 게 맞다면? 파싱!
            if (rxMsg.id == CAN_ID_ENGINE_RPM && rxMsg.dlc >= 8)
            {
                canRxCount++;

                rx_rpm = SignalDb_DecodeMotor1Rpm(rxMsg.data);

                HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
            }
        }
    }
}
