#include "cmsis_os.h"
#include "can_bsp.h"
#include "usart.h"

#include <stdarg.h>
#include <stdio.h>

extern CAN_HandleTypeDef hcan2;

static osMutexId_t s_uart_mutex = NULL;
static volatile uint8_t s_warning_active = 0U;

static void log_printf(const char *fmt, ...)
{
    if (s_uart_mutex == NULL) {
        return;
    }

    char buf[128];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len <= 0) {
        return;
    }
    if (len > (int)sizeof(buf)) {
        len = (int)sizeof(buf);
    }

    osMutexAcquire(s_uart_mutex, osWaitForever);
    (void)HAL_UART_Transmit(&huart3, (uint8_t *)buf, (uint16_t)len, 100U);
    osMutexRelease(s_uart_mutex);
}

void StartDefaultTask(void *argument)
{
    (void)argument;

    if (s_uart_mutex == NULL) {
        s_uart_mutex = osMutexNew(NULL);
    }

    if (CAN_BSP_Init() == HAL_OK) {
        log_printf("\r\n[GW] Gateway init complete\r\n");
    } else {
        log_printf("\r\n[GW] Gateway init failed\r\n");
    }

    for (;;) {
        osDelay(1000U);
    }
}

void GatewayTask(void *argument)
{
    (void)argument;

    for (;;) {
        CAN_RxMessage_t rxMsg;

        if (!CAN_BSP_Read(&rxMsg, osWaitForever)) {
            continue;
        }

        if (rxMsg.bus == 1U && rxMsg.id == CAN_ID_ENGINE_DATA) {
            uint16_t rpm = CAN_GetU16LE(rxMsg.data, CAN_ENGINE_DATA_RPM_IDX);
            HAL_StatusTypeDef tx_status =
                CAN_BSP_SendTo(&hcan2, rxMsg.id, rxMsg.data, rxMsg.dlc);

            if (rpm >= 5000U) {
                s_warning_active = 1U;
            } else if (rpm < 4500U) {
                s_warning_active = 0U;
            }

            if (tx_status != HAL_OK) {
                log_printf("[GW] CAN2 TX fail id=0x%03lX st=%d\r\n",
                           (unsigned long)rxMsg.id,
                           (int)tx_status);
            }
        }
    }
}

void ClusterTask(void *argument)
{
    (void)argument;

    for (;;) {
        osDelay(10U);
    }
}

void LoggerTask(void *argument)
{
    (void)argument;

    for (;;) {
        log_printf("[GW] RX1=%lu TX1=%lu RX2=%lu TX2=%lu busy=%lu err=%lu warn=%u\r\n",
                   (unsigned long)can1RxCount,
                   (unsigned long)can1TxCount,
                   (unsigned long)can2RxCount,
                   (unsigned long)can2TxCount,
                   (unsigned long)canTxBusyCount,
                   (unsigned long)canTxErrorCount,
                   (unsigned int)s_warning_active);
        osDelay(1000U);
    }
}
