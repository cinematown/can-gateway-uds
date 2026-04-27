#ifndef CLI_CMD_H
#define CLI_CMD_H

#include "cli.h"
#include "engine_sim.h"
#include "can_driver.h"
#include "cmsis_os2.h"
#include "uart.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void CliCmd_Init(void);

#endif