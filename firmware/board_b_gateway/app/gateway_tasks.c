#include "cmsis_os.h"
#include "can_bsp.h"
#include "can_cli_monitor.h"
#include "cli.h"
#include "gateway_body_bridge.h"
#include "gateway_engine_bridge.h"
#include "uart.h"

#include <stdarg.h>
#include <stdio.h>

extern CAN_HandleTypeDef hcan2;

static volatile uint8_t s_warning_active = 0U;
static volatile uint8_t s_gateway_ready = 0U;
static volatile uint8_t s_status_log_enabled = 1U;

static void log_printf(const char *fmt, ...)
{
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

    (void)uartWrite(0, (uint8_t *)buf, (uint32_t)len);
}

static void log_cmd(uint8_t argc, char *argv[])
{
    if (argc == 1U || strcmp(argv[1], "stat") == 0) {
        cliPrintf("status log: %s\r\n", s_status_log_enabled ? "on" : "off");
        return;
    }

    if (strcmp(argv[1], "on") == 0) {
        s_status_log_enabled = 1U;
        cliPrintf("status log on\r\n");
        return;
    }

    if (strcmp(argv[1], "off") == 0) {
        s_status_log_enabled = 0U;
        cliPrintf("status log off\r\n");
        return;
    }

    cliPrintf("usage: log [on|off|stat]\r\n");
}

void StartDefaultTask(void *argument)
{
    (void)argument;

    if (uartInit()) {
        cliInit();
        (void)cliAdd("log", log_cmd);
        CanCliMonitor_Init();
        cliPrintf("\r\n[GW] UART CLI ready. type 'help', 'log off', or 'canlog stat'\r\n");
    }

    if (CAN_BSP_Init() == HAL_OK) {
        s_gateway_ready = 1U;
        log_printf("\r\n[GW] Gateway init complete\r\n");
    } else {
        log_printf("\r\n[GW] Gateway init failed\r\n");
    }

    cliPrintf("CLI > ");

    for (;;) {
        cliMain();
        osDelay(1U);
    }
}

void GatewayTask(void *argument)
{
    (void)argument;

    while (s_gateway_ready == 0U) {
        osDelay(10U);
    }

    for (;;) {
        CAN_RxMessage_t rxMsg;

        if (!CAN_BSP_Read(&rxMsg, osWaitForever)) {
            osDelay(1U);
            continue;
        }

        CanCliMonitor_LogRx(&rxMsg);
        GatewayEngineBridge_OnRx(&rxMsg);
        GatewayBodyBridge_OnRx(&rxMsg);

        if (rxMsg.bus == 1U && rxMsg.id == CAN_ID_ENGINE_DATA) {
            uint16_t rpm = CAN_GetU16LE(rxMsg.data, CAN_ENGINE_DATA_RPM_IDX);
            HAL_StatusTypeDef tx_status =
                CAN_BSP_SendTo(&hcan2, rxMsg.id, rxMsg.data, rxMsg.dlc);
            CanCliMonitor_LogTx(2U, rxMsg.id, rxMsg.data, rxMsg.dlc, tx_status);

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

    while (s_gateway_ready == 0U) {
        osDelay(10U);
    }

    for (;;) {
        GatewayEngineBridge_Task10ms();
        GatewayBodyBridge_Task10ms();
        osDelay(10U);
    }
}

void LoggerTask(void *argument)
{
    (void)argument;

    while (s_gateway_ready == 0U) {
        osDelay(10U);
    }

    for (;;) {
        if (s_status_log_enabled != 0U) {
            log_printf("[GW] RX1=%lu TX1=%lu RX2=%lu TX2=%lu busy=%lu err=%lu warn=%u\r\n",
                       (unsigned long)can1RxCount,
                       (unsigned long)can1TxCount,
                       (unsigned long)can2RxCount,
                       (unsigned long)can2TxCount,
                       (unsigned long)canTxBusyCount,
                       (unsigned long)canTxErrorCount,
                       (unsigned int)s_warning_active);
        }
        osDelay(1000U);
    }
}
