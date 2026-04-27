#include "cli_cmd.h"

static uint32_t parseNumber(const char *str)
{
    if (str == NULL)
        return 0;

    if ((str[0] == '0') && (str[1] == 'x' || str[1] == 'X'))
    {
        return strtoul(str, NULL, 16);
    }

    return strtoul(str, NULL, 10);
}

static void cmdMode(uint8_t argc, char *argv[])
{
    if (argc < 2)
    {
        cliPrintf("usage: mode adc|uart|raw\r\n");
        return;
    }

    if (strcmp(argv[1], "adc") == 0)
    {
        EngineSim_SetMode(ENGINE_MODE_ADC);
        cliPrintf("mode = adc\r\n");
    }
    else if (strcmp(argv[1], "uart") == 0)
    {
        EngineSim_SetMode(ENGINE_MODE_UART);
        cliPrintf("mode = uart\r\n");
    }
    else if (strcmp(argv[1], "raw") == 0)
    {
        EngineSim_SetMode(ENGINE_MODE_RAW);
        cliPrintf("mode = raw\r\n");
    }
    else
    {
        cliPrintf("invalid mode\r\n");
    }
}

static void cmdThrottle(uint8_t argc, char *argv[])
{
    if (argc < 2)
    {
        cliPrintf("usage: throttle 0~100\r\n");
        return;
    }

    uint32_t value = parseNumber(argv[1]);

    if (value > 100)
        value = 100;

    EngineSim_SetMode(ENGINE_MODE_UART);
    EngineSim_SetThrottle((uint8_t)value);

    cliPrintf("mode = uart\r\n");
    cliPrintf("throttle = %lu\r\n", value);
}

static void cmdBrake(uint8_t argc, char *argv[])
{
    if (argc < 2)
    {
        cliPrintf("usage: brake 0~100\r\n");
        return;
    }

    uint32_t value = parseNumber(argv[1]);

    if (value > 100)
        value = 100;

    EngineSim_SetBrake((uint8_t)value);

    cliPrintf("brake = %lu\r\n", value);
}

static void cmdStatus(uint8_t argc, char *argv[])
{
    (void)argc;
    (void)argv;

    EngineSimStatus_t status;

    EngineSim_GetStatus(&status);

    cliPrintf("\r\n[EngineSim Status]\r\n");
    cliPrintf("mode     : %s\r\n", EngineSim_GetModeString(status.mode));
    cliPrintf("throttle : %d\r\n", status.throttle);
    cliPrintf("brake    : %d\r\n", status.brake);
    cliPrintf("rpm      : %d\r\n", status.rpm);
    cliPrintf("speed    : %d\r\n", status.speed);
    cliPrintf("coolant  : %d\r\n", status.coolant);
    cliPrintf("tx_count : %lu\r\n", status.tx_count);
}

static void cmdRaw(uint8_t argc, char *argv[])
{
    if (argc < 4)
    {
        cliPrintf("usage: raw id dlc data...\r\n");
        cliPrintf("ex: raw 0x100 8 A6 04 1D 00 51 20 00 00\r\n");
        return;
    }

    uint32_t id = parseNumber(argv[1]);
    uint8_t dlc = (uint8_t)parseNumber(argv[2]);

    if (dlc > 8)
    {
        cliPrintf("dlc must be 0~8\r\n");
        return;
    }

    if (argc < 3 + dlc)
    {
        cliPrintf("not enough data bytes\r\n");
        return;
    }

    uint8_t data[8] = {0};

    for (uint8_t i = 0; i < dlc; i++)
    {
        // [수정] 데이터 페이로드는 0x 유무와 상관없이 무조건 16진수로 파싱!
        data[i] = (uint8_t)strtoul(argv[3 + i], NULL, 16); 
    }

    EngineSim_SetMode(ENGINE_MODE_RAW);

    HAL_StatusTypeDef ret = CAN1_Send(id, data, dlc);

    if (ret == HAL_OK)
    {
        cliPrintf("raw can tx ok\r\n");
    }
    else
    {
        cliPrintf("raw can tx fail: %d\r\n", ret);
    }
}

static void cmdWatch(uint8_t argc, char *argv[])
{
    uint32_t period = 500;
    uint8_t rx_data;

    if (argc >= 2)
    {
        period = parseNumber(argv[1]);

        if (period < 100)
            period = 100;
    }

    cliPrintf("EngineSim watch start. Press Ctrl+C to stop.\r\n");

    while (1)
    {
        if (uartReadBlock(0, &rx_data, 0) == true)
        {
            if (rx_data == 0x03)
            {
                cliPrintf("\r\nwatch stop\r\nCLI>");
                return;
            }
        }

        EngineSimStatus_t status;

        EngineSim_GetStatus(&status);

        cliPrintf("\033[2J\033[H");
        cliPrintf("[EngineSim Live Status]\r\n");
        cliPrintf("===========================\r\n");
        cliPrintf("mode     : %s\r\n", EngineSim_GetModeString(status.mode));
        cliPrintf("throttle : %d %%\r\n", status.throttle);
        cliPrintf("brake    : %d %%\r\n", status.brake);
        cliPrintf("rpm      : %d\r\n", status.rpm);
        cliPrintf("speed    : %d\r\n", status.speed);
        cliPrintf("coolant  : %d\r\n", status.coolant);
        cliPrintf("tx_count : %lu\r\n", status.tx_count);

        cliPrintf("\r\n[CAN TX]\r\n");
        cliPrintf("can1TxCount       : %lu\r\n", can1TxCount);
        cliPrintf("canSendEnterCount : %lu\r\n", canSendEnterCount);
        cliPrintf("canTxBusyCount    : %lu\r\n", canTxBusyCount);
        cliPrintf("canTxErrorCount   : %lu\r\n", canTxErrorCount);

        // cliPrintf("\r\n[CAN2 RX]\r\n");
        // cliPrintf("can2RxCount : %lu\r\n", can2RxCount);
        // cliPrintf("rx_rpm      : %d\r\n", rx_rpm);
        // cliPrintf("rx_speed    : %d\r\n", rx_speed);
        // cliPrintf("rx_coolant  : %d\r\n", rx_coolant);
        // cliPrintf("rx_throttle : %d\r\n", rx_throttle);
        // cliPrintf("rx_brake    : %d\r\n", rx_brake);

        cliPrintf("\r\nPress Ctrl+C to stop.\r\n");

        osDelay(period);
    }
}

void CliCmd_Init(void)
{
    cliAdd("mode", cmdMode);
    cliAdd("throttle", cmdThrottle);
    cliAdd("brake", cmdBrake);
    cliAdd("status", cmdStatus);
    cliAdd("raw", cmdRaw);
    cliAdd("watch", cmdWatch);
}