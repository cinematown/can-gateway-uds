#include "uds_cli_cmd.h"

#include "cli.h"
#include "protocol_ids.h"
#include "uds_service.h"
#include <string.h>

static void cmdRead(uint8_t argc, char **argv)
{
    if (argc < 2) {
        cliPrintf("Usage: read vin|rpm|speed|temp\r\n");
        return;
    }

    if (strcmp(argv[1], "vin") == 0) {
        UDS_Execute_Diagnostic(UDS_DID_VIN, "VIN");
    } else if (strcmp(argv[1], "rpm") == 0) {
        UDS_Execute_Diagnostic(UDS_DID_RPM, "RPM");
    } else if (strcmp(argv[1], "speed") == 0) {
        UDS_Execute_Diagnostic(UDS_DID_SPEED, "SPEED");
    } else if (strcmp(argv[1], "temp") == 0) {
        UDS_Execute_Diagnostic(UDS_DID_TEMP, "TEMP");
    } else {
        cliPrintf("Unknown item: %s\r\n", argv[1]);
    }
}

void UDS_CLI_Init(void)
{
    (void)cliAdd("read", cmdRead);
}
