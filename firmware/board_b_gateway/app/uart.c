#include "uart.h"

extern UART_HandleTypeDef huart3;

static osMessageQueueId_t uart_rx_q = NULL;
static osMutexId_t uart_tx_mutex = NULL;

#define TIMEOUT 100

#define UART_RX_BUF_LENGTH 256

static uint8_t rx_data;

bool uartInit(void) {

    if (uart_rx_q == NULL) {
        uart_rx_q = osMessageQueueNew(UART_RX_BUF_LENGTH, sizeof(uint8_t), NULL);
    }

    if (uart_tx_mutex == NULL) {
        uart_tx_mutex = osMutexNew(NULL);
    }

    bool ret = uartOpen(0, 115200);

    if (ret) {
        HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART3_IRQn);
        HAL_UART_Receive_IT(&huart3, &rx_data, 1);
    }
    return ret;
}

void USART3_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart3);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    if (huart->Instance == USART3) {
        if (uart_rx_q != NULL) {
            osMessageQueuePut(uart_rx_q, &rx_data, 0, 0);
        }
        HAL_UART_Receive_IT(&huart3, &rx_data, 1);
    }
}

uint32_t uartAvailable(uint8_t ch) {
    if (ch == 0 && uart_rx_q != NULL) {
        return osMessageQueueGetCount(uart_rx_q);
    }
    return 0;
}

uint8_t uartRead(uint8_t ch) {
    uint8_t ret = 0;
    if (ch == 0 && uart_rx_q != NULL) {
        osMessageQueueGet(uart_rx_q, &ret, NULL, 0);
    }
    return ret;
}

bool uartReadBlock(uint8_t ch, uint8_t* p_data, uint32_t timeout) {
    if (ch == 0 && uart_rx_q != NULL) {
        if (osMessageQueueGet(uart_rx_q, p_data, NULL, timeout) == osOK)
            return true;
    }
    return false;
}

bool uartOpen(uint8_t ch, uint32_t baudrate)
{
    if (ch != 0)
        return false;

    if (baudrate != huart3.Init.BaudRate)
        return false;

    return true;
}

bool uartClose(uint8_t ch) {
    return true;
}

uint32_t uartWrite(uint8_t ch, uint8_t* p_data, uint32_t len) {
    if (uart_tx_mutex == NULL) return 0;

    osMutexAcquire(uart_tx_mutex, osWaitForever);

    HAL_StatusTypeDef status = HAL_UART_Transmit(&huart3, p_data, len, TIMEOUT);

    osMutexRelease(uart_tx_mutex);

    if (status == HAL_OK) {
        return len; // 성공 시 전송한 길이 반환
    }
    else {
        return 0;   // 실패 시 0 반환 (에러 처리용)
    }
}

uint32_t uartPrintf(uint8_t ch, const char* fmt, ...)
{
    char buf[128];
    va_list args;

    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len < 0)
        return 0;

    if (len >= sizeof(buf))
        len = sizeof(buf) - 1;

    return uartWrite(ch, (uint8_t*)buf, len);
}
