#include "can_cli_monitor.h"

#include "cli.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern CAN_HandleTypeDef hcan1;
#ifdef BOARD_B_GATEWAY
extern CAN_HandleTypeDef hcan2;
#endif

static bool s_canlog_enabled = false;
static bool s_filter_enabled = false;
static uint32_t s_filter_id = 0U;

static uint32_t s_rx_log_count = 0U;
static uint32_t s_tx_log_count = 0U;
static uint32_t s_drop_count = 0U;

static bool id_matches(uint32_t id)
{
    return !s_filter_enabled || id == s_filter_id;
}

static void print_frame_prefix(const char *dir, uint8_t bus, uint32_t id,
                               uint8_t dlc)
{
    cliPrintf("[%s%u] id=0x%03lX dlc=%u data=",
              dir,
              (unsigned int)bus,
              (unsigned long)id,
              (unsigned int)dlc);
}

static void print_data(const uint8_t *data, uint8_t dlc)
{
    if (data == NULL) {
        cliPrintf("(null)");
        return;
    }

    for (uint8_t i = 0U; i < dlc && i < 8U; i++) {
        cliPrintf("%02X", (unsigned int)data[i]);
        if (i + 1U < dlc && i + 1U < 8U) {
            cliPrintf(" ");
        }
    }
}

static void print_status(void)
{
    cliPrintf("canlog: %s, filter: ",
              s_canlog_enabled ? "on" : "off");

    if (s_filter_enabled) {
        cliPrintf("0x%03lX", (unsigned long)s_filter_id);
    } else {
        cliPrintf("all");
    }

    cliPrintf("\r\n");
    cliPrintf("counts: rx_log=%lu tx_log=%lu drop=%lu\r\n",
              (unsigned long)s_rx_log_count,
              (unsigned long)s_tx_log_count,
              (unsigned long)s_drop_count);
    cliPrintf("bsp: RX1=%lu TX1=%lu RX2=%lu TX2=%lu busy=%lu err=%lu\r\n",
              (unsigned long)can1RxCount,
              (unsigned long)can1TxCount,
              (unsigned long)can2RxCount,
              (unsigned long)can2TxCount,
              (unsigned long)canTxBusyCount,
              (unsigned long)canTxErrorCount);
    cliPrintf("init: step=%lu err_step=%lu (8/0 means OK)\r\n",
              (unsigned long)canInitStep,
              (unsigned long)canInitErrorStep);
    cliPrintf("can1: state=%lu err=0x%08lX MCR=0x%08lX MSR=0x%08lX\r\n",
              (unsigned long)hcan1.State,
              (unsigned long)HAL_CAN_GetError(&hcan1),
              (unsigned long)hcan1.Instance->MCR,
              (unsigned long)hcan1.Instance->MSR);
#ifdef BOARD_B_GATEWAY
    cliPrintf("can2: state=%lu err=0x%08lX MCR=0x%08lX MSR=0x%08lX\r\n",
              (unsigned long)hcan2.State,
              (unsigned long)HAL_CAN_GetError(&hcan2),
              (unsigned long)hcan2.Instance->MCR,
              (unsigned long)hcan2.Instance->MSR);
#endif
}

static uint32_t parse_id(const char *text)
{
    if (text == NULL) {
        return 0U;
    }

    if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        return (uint32_t)strtoul(text, NULL, 16);
    }

    return (uint32_t)strtoul(text, NULL, 16);
}

static void canlog_cmd(uint8_t argc, char *argv[])
{
    if (argc == 1U || strcmp(argv[1], "stat") == 0) {
        print_status();
        return;
    }

    if (strcmp(argv[1], "on") == 0) {
        s_canlog_enabled = true;
        cliPrintf("canlog on\r\n");
        return;
    }

    if (strcmp(argv[1], "off") == 0) {
        s_canlog_enabled = false;
        cliPrintf("canlog off\r\n");
        return;
    }

    if (strcmp(argv[1], "all") == 0) {
        s_filter_enabled = false;
        cliPrintf("canlog filter all\r\n");
        return;
    }

    if (strcmp(argv[1], "id") == 0) {
        if (argc < 3U) {
            cliPrintf("usage: canlog id <hex_id|all>  ex) 100, 280, 1A0\r\n");
            return;
        }

        if (strcmp(argv[2], "all") == 0) {
            s_filter_enabled = false;
            cliPrintf("canlog filter all\r\n");
            return;
        }

        s_filter_id = parse_id(argv[2]);
        s_filter_enabled = true;
        cliPrintf("canlog filter 0x%03lX\r\n", (unsigned long)s_filter_id);
        return;
    }

    if (strcmp(argv[1], "clear") == 0) {
        s_rx_log_count = 0U;
        s_tx_log_count = 0U;
        s_drop_count = 0U;
        cliPrintf("canlog counters cleared\r\n");
        return;
    }

    cliPrintf("usage: canlog [on|off|stat|all|id <hex_id|all>|clear]\r\n");
}

void CanCliMonitor_Init(void)
{
    s_canlog_enabled = false;
    s_filter_enabled = false;
    s_filter_id = 0U;
    s_rx_log_count = 0U;
    s_tx_log_count = 0U;
    s_drop_count = 0U;

    (void)cliAdd("canlog", canlog_cmd);
}

void CanCliMonitor_LogRx(const CAN_RxMessage_t *rx_msg)
{
    if (!s_canlog_enabled || rx_msg == NULL || !id_matches(rx_msg->id)) {
        return;
    }

    print_frame_prefix("RX", rx_msg->bus, rx_msg->id, rx_msg->dlc);
    print_data(rx_msg->data, rx_msg->dlc);
    cliPrintf("\r\n");
    s_rx_log_count++;
}

void CanCliMonitor_LogTx(uint8_t bus, uint32_t id, const uint8_t *data,
                         uint8_t dlc, HAL_StatusTypeDef status)
{
    if (!s_canlog_enabled || !id_matches(id)) {
        return;
    }

    print_frame_prefix("TX", bus, id, dlc);
    print_data(data, dlc);
    cliPrintf(" st=%d\r\n", (int)status);

    if (status == HAL_OK) {
        s_tx_log_count++;
    } else {
        s_drop_count++;
    }
}
