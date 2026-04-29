#ifndef UART_H
#define UART_H

#include "hw_def.h"
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

/* --- 함수 선언 --- */

/**
 * @brief UART 자원(Queue, Mutex) 초기화 및 인터럽트 수신 시작
 */
bool uartInit(void);

/**
 * @brief UART 포트 열기 및 보드레이트 설정
 */
bool uartOpen(uint8_t ch, uint32_t baudrate);

/**
 * @brief UART 포트 닫기
 */
bool uartClose(uint8_t ch);

/**
 * @brief 수신 버퍼(Queue)에 쌓인 데이터 개수 확인
 */
uint32_t uartAvailable(uint8_t ch);

/**
 * @brief 수신 버퍼에서 데이터 1바이트 읽기 (Non-blocking)
 */
uint8_t uartRead(uint8_t ch);

/**
 * @brief 수신 버퍼에서 데이터 읽기 (Blocking, Timeout 설정 가능)
 */
bool uartReadBlock(uint8_t ch, uint8_t *p_data, uint32_t timeout);

/**
 * @brief 데이터를 외부로 전송 (Mutex 보호 포함)
 */
uint32_t uartWrite(uint8_t ch, uint8_t *p_data, uint32_t len);

/**
 * @brief 서식화된 문자열 출력 (printf 스타일)
 */
uint32_t uartPrintf(uint8_t ch, const char *fmt, ...);

#endif /* UART_H */