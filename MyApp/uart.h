#ifndef UART_H
#define UART_H

#include "main.h"
#include "cmsis_os2.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

bool uartInit(void);
bool uartOpen(uint8_t ch, uint32_t baudrate);
bool uartClose(uint8_t ch);

uint32_t uartAvailable(uint8_t ch);
uint8_t uartRead(uint8_t ch);
bool uartReadBlock(uint8_t ch, uint8_t *p_data, uint32_t timeout);

uint32_t uartWrite(uint8_t ch, uint8_t *p_data, uint32_t len);
uint32_t uartPrintf(uint8_t ch, const char *fmt, ...);

#endif