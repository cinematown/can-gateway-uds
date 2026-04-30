#include "cli_cmd.h"
#include "cli.h"
#include "uds_client.h"
#include "can_bsp.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*
 * Board C CLI - Cluster UDS Diagnostic Client
 *
 * 목적:
 * - Board C가 CAN1로 실제 계기판에 UDS 요청을 송신
 * - 계기판 응답(기본 0x77E)을 UART CLI에서 확인
 * - 자주 쓰는 DID를 전용 명령으로 제공
 */

static bool parse_hex32(const char *s, uint32_t *out)
{
    char *end = NULL;
    unsigned long v;

    if (s == NULL || out == NULL)
    {
        return false;
    }

    v = strtoul(s, &end, 16);
    if (*end != '\0' || v > 0xFFFFFFFFul)
    {
        return false;
    }

    *out = (uint32_t)v;
    return true;
}

static bool parse_hex16(const char *s, uint16_t *out)
{
    uint32_t v;

    if (!parse_hex32(s, &v) || v > 0xFFFFu)
    {
        return false;
    }

    *out = (uint16_t)v;
    return true;
}

static bool parse_hex8(const char *s, uint8_t *out)
{
    uint32_t v;

    if (!parse_hex32(s, &v) || v > 0xFFu)
    {
        return false;
    }

    *out = (uint8_t)v;
    return true;
}

static void cmd_cluster_info(uint8_t argc, char *argv[])
{
    UDS_ClientStatus_t c;
    (void)argc;
    (void)argv;

    UDS_Client_GetStatus(&c);

    cliPrintf("\r\n[Board C - Cluster UDS Diagnostic Client]\r\n");
    cliPrintf("CAN Bus             : CAN1\r\n");
    cliPrintf("Cluster Req CAN ID  : 0x%03lX  (Board C -> Cluster)\r\n", (unsigned long)c.req_id);
    cliPrintf("Cluster Resp CAN ID : 0x%03lX  (Cluster -> Board C)\r\n", (unsigned long)c.resp_id);

    cliPrintf("\r\n[Dedicated UDS DID Requests]\r\n");
    cliPrintf("VIN      : %04X  -> read vin\r\n",    UDS_DID_VIN);
    cliPrintf("Part No. : %04X  -> read part\r\n",   UDS_DID_PART_NUMBER);
    cliPrintf("SW No.   : %04X  -> read sw\r\n",     UDS_DID_SW_NUMBER);
    cliPrintf("SW Ver.  : %04X  -> read swver\r\n",  UDS_DID_SW_VERSION);
    cliPrintf("Serial   : %04X  -> read serial\r\n", UDS_DID_SERIAL_NUMBER);
    cliPrintf("HW No.   : %04X  -> read hw\r\n",     UDS_DID_HW_NUMBER);
    cliPrintf("System   : %04X  -> read system\r\n", UDS_DID_SYSTEM_NAME);

    cliPrintf("\r\n[Optional signal DID Requests]\r\n");
    cliPrintf("RPM      : %04X  -> read rpm\r\n",    UDS_DID_RPM);
    cliPrintf("Speed    : %04X  -> read speed\r\n",  UDS_DID_SPEED);
    cliPrintf("Coolant  : %04X  -> read temp\r\n",   UDS_DID_COOLANT);

    // 나머지 status 출력은 동일
    cliPrintf("\r\n[Status]\r\n");
    cliPrintf("Last DID : 0x%04X\r\n", c.last_did);
    cliPrintf("Tx       : %lu\r\n", (unsigned long)c.tx_count);
    cliPrintf("Rx       : %lu\r\n", (unsigned long)c.rx_count);
    cliPrintf("Error    : %lu\r\n", (unsigned long)c.err_count);
    cliPrintf("CAN Log  : %s\r\n",  c.log_on ? "on" : "off");
}

static void cmd_read_cluster(uint8_t argc, char *argv[])
{
    if (argc != 2u)
    {
        cliPrintf("usage: read vin|part|sw|swver|serial|hw|system|rpm|speed|temp|all\r\n");
        cliPrintf("ex   : read vin\r\n");
        cliPrintf("ex   : read part\r\n");
        return;
    }

    if (strcmp(argv[1], "vin") == 0)
    {
        UDS_Execute_Diagnostic(UDS_DID_VIN, "VIN");
    }
    else if (strcmp(argv[1], "part") == 0 || strcmp(argv[1], "partno") == 0)
    {
        UDS_Execute_Diagnostic(UDS_DID_PART_NUMBER, "PART NUMBER");
    }
    else if (strcmp(argv[1], "sw") == 0 || strcmp(argv[1], "swnum") == 0)
    {
        UDS_Execute_Diagnostic(UDS_DID_SW_NUMBER, "SW NUMBER");
    }
    else if (strcmp(argv[1], "swver") == 0 || strcmp(argv[1], "swversion") == 0)
    {
        UDS_Execute_Diagnostic(UDS_DID_SW_VERSION, "SW VERSION");
    }
    else if (strcmp(argv[1], "serial") == 0 || strcmp(argv[1], "sn") == 0)
    {
        UDS_Execute_Diagnostic(UDS_DID_SERIAL_NUMBER, "ECU SERIAL");
    }
    else if (strcmp(argv[1], "hw") == 0 || strcmp(argv[1], "hwnum") == 0)
    {
        UDS_Execute_Diagnostic(UDS_DID_HW_NUMBER, "HW NUMBER");
    }
    else if (strcmp(argv[1], "system") == 0 || strcmp(argv[1], "sys") == 0)
    {
        UDS_Execute_Diagnostic(UDS_DID_SYSTEM_NAME, "SYSTEM NAME");
    }
    else if (strcmp(argv[1], "rpm") == 0)
    {
        UDS_Execute_Diagnostic(UDS_DID_RPM, "RPM");
    }
    else if (strcmp(argv[1], "speed") == 0)
    {
        UDS_Execute_Diagnostic(UDS_DID_SPEED, "SPEED");
    }
    else if (strcmp(argv[1], "temp") == 0 || strcmp(argv[1], "coolant") == 0)
    {
        UDS_Execute_Diagnostic(UDS_DID_COOLANT, "TEMP");
    }
    else if (strcmp(argv[1], "all") == 0)
    {
        UDS_Execute_Diagnostic(UDS_DID_ALL, "ALL");
    }
    else
    {
        cliPrintf("unknown target: %s\r\n", argv[1]);
        cliPrintf("usage: read vin|part|sw|swver|serial|hw|system|rpm|speed|temp\r\n");
    }
}

static void cmd_cl_target(uint8_t argc, char *argv[])
{
    uint32_t req_id;
    uint32_t resp_id;
    UDS_ClientStatus_t c;

    if (argc == 1u)
    {
        UDS_Client_GetStatus(&c);
        cliPrintf("Cluster target: req=0x%03lX resp=0x%03lX\r\n",
                  (unsigned long)c.req_id,
                  (unsigned long)c.resp_id);
        cliPrintf("usage: cl_target <req_id> <resp_id>\r\n");
        cliPrintf("ex   : cl_target 714 77E\r\n");
        return;
    }

    if (argc != 3u || !parse_hex32(argv[1], &req_id) || !parse_hex32(argv[2], &resp_id) ||
        req_id > 0x7FFu || resp_id > 0x7FFu)
    {
        cliPrintf("usage: cl_target <req_id> <resp_id>\r\n");
        cliPrintf("ex   : cl_target 714 77E\r\n");
        return;
    }

    UDS_Client_SetTarget(req_id, resp_id);
    cliPrintf("Cluster target set: req=0x%03lX resp=0x%03lX\r\n",
              (unsigned long)req_id,
              (unsigned long)resp_id);
}

static void cmd_cl_read(uint8_t argc, char *argv[])
{
    uint16_t did;

    if (argc != 2u || !parse_hex16(argv[1], &did))
    {
        cliPrintf("usage: cl_read <did>\r\n");
        cliPrintf("ex   : cl_read F190\r\n");
        cliPrintf("ex   : cl_read F187\r\n");
        return;
    }

    (void)UDS_Client_ReadDID(did);
}

static void cmd_cl_vin(uint8_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    UDS_Execute_Diagnostic(UDS_DID_VIN, "VIN");
}

static void cmd_cl_part(uint8_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    UDS_Execute_Diagnostic(UDS_DID_PART_NUMBER, "PART NUMBER");
}

static void cmd_cl_sw(uint8_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    UDS_Execute_Diagnostic(UDS_DID_SW_NUMBER, "SW NUMBER");
}

static void cmd_cl_swver(uint8_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    UDS_Execute_Diagnostic(UDS_DID_SW_VERSION, "SW VERSION");
}

static void cmd_cl_serial(uint8_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    UDS_Execute_Diagnostic(UDS_DID_SERIAL_NUMBER, "ECU SERIAL");
}

static void cmd_cl_hw(uint8_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    UDS_Execute_Diagnostic(UDS_DID_HW_NUMBER, "HW NUMBER");
}

static void cmd_cl_system(uint8_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    UDS_Execute_Diagnostic(UDS_DID_SYSTEM_NAME, "SYSTEM NAME");
}

static void cmd_cl_rpm(uint8_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    UDS_Execute_Diagnostic(UDS_DID_RPM, "RPM");
}

static void cmd_cl_speed(uint8_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    UDS_Execute_Diagnostic(UDS_DID_SPEED, "SPEED");
}

static void cmd_cl_cool(uint8_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    UDS_Execute_Diagnostic(UDS_DID_COOLANT, "TEMP");
}

static void cmd_can_log(uint8_t argc, char *argv[])
{
    if (argc != 2u)
    {
        cliPrintf("usage: can_log on|off\r\n");
        cliPrintf("current: %s\r\n", UDS_Client_GetCanLog() ? "on" : "off");
        return;
    }

    if (strcmp(argv[1], "on") == 0)
    {
        UDS_Client_SetCanLog(true);
        cliPrintf("CAN1 RX log ON\r\n");
    }
    else if (strcmp(argv[1], "off") == 0)
    {
        UDS_Client_SetCanLog(false);
        cliPrintf("CAN1 RX log OFF\r\n");
    }
    else
    {
        cliPrintf("usage: can_log on|off\r\n");
    }
}

static void cmd_can_tx(uint8_t argc, char *argv[])
{
    uint32_t id;
    uint8_t data[8] = {0};
    uint8_t len;
    HAL_StatusTypeDef ret;

    if (argc < 3u || argc > 10u || !parse_hex32(argv[1], &id) || id > 0x7FFu)
    {
        cliPrintf("usage: can_tx <id> <b0> [b1] ... [b7]\r\n");
        cliPrintf("ex   : can_tx 714 03 22 F1 90 00 00 00 00\r\n");
        return;
    }

    len = (uint8_t)(argc - 2u);
    for (uint8_t i = 0u; i < len; i++)
    {
        if (!parse_hex8(argv[i + 2u], &data[i]))
        {
            cliPrintf("invalid byte: %s\r\n", argv[i + 2u]);
            return;
        }
    }

    ret = CAN_BSP_Send(id, data, len);
    cliPrintf("[CAN1 TX] ID=0x%03lX DLC=%u ret=%d DATA=",
              (unsigned long)id,
              (unsigned int)len,
              (int)ret);
    for (uint8_t i = 0u; i < len; i++)
    {
        cliPrintf("%02X ", data[i]);
    }
    cliPrintf("\r\n");
}

static void cmd_can_stat(uint8_t argc, char *argv[])
{
    UDS_ClientStatus_t c;
    (void)argc;
    (void)argv;

    UDS_Client_GetStatus(&c);

    cliPrintf("[CAN BSP]\r\n");
    cliPrintf("can1TxCount       : %lu\r\n", (unsigned long)can1TxCount);
    cliPrintf("can1RxCount       : %lu\r\n", (unsigned long)can1RxCount);
    cliPrintf("canSendEnterCount : %lu\r\n", (unsigned long)canSendEnterCount);
    cliPrintf("canTxBusyCount    : %lu\r\n", (unsigned long)canTxBusyCount);
    cliPrintf("canTxErrorCount   : %lu\r\n", (unsigned long)canTxErrorCount);
    cliPrintf("[Cluster Client]\r\n");
    cliPrintf("req=0x%03lX resp=0x%03lX lastDid=0x%04X\r\n",
              (unsigned long)c.req_id,
              (unsigned long)c.resp_id,
              c.last_did);
    cliPrintf("tx=%lu rx=%lu err=%lu log=%s\r\n",
              (unsigned long)c.tx_count,
              (unsigned long)c.rx_count,
              (unsigned long)c.err_count,
              c.log_on ? "on" : "off");
}

void CLI_CMD_Init(void)
{
    /* 계기판 UDS Client 명령 */
    cliAdd("cluster_info", cmd_cluster_info);
    cliAdd("read", cmd_read_cluster);

    /* 직접 DID/ID를 지정해서 계기판에 요청 */
    cliAdd("cl_target", cmd_cl_target);
    cliAdd("cl_read", cmd_cl_read);
    cliAdd("cl_vin", cmd_cl_vin);
    cliAdd("cl_part", cmd_cl_part);
    cliAdd("cl_sw", cmd_cl_sw);
    cliAdd("cl_swver", cmd_cl_swver);
    cliAdd("cl_serial", cmd_cl_serial);
    cliAdd("cl_hw", cmd_cl_hw);
    cliAdd("cl_system", cmd_cl_system);
    cliAdd("cl_rpm", cmd_cl_rpm);
    cliAdd("cl_speed", cmd_cl_speed);
    cliAdd("cl_cool", cmd_cl_cool);

    /* CAN1 통신 확인용 명령 */
    cliAdd("can_log", cmd_can_log);
    cliAdd("can_tx", cmd_can_tx);
    cliAdd("can_stat", cmd_can_stat);
}
