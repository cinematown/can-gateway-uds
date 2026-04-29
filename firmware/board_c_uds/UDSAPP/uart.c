#include "uart.h"

extern UART_HandleTypeDef huart3; // Board C에서 사용하는 핸들러 확인 (2번 맞죠?)

static osMessageQueueId_t uart_rx_q = NULL;
static osMutexId_t uart_tx_mutex = NULL;

#define UART_RX_BUF_LENGTH 256
static uint8_t rx_data;

bool uartInit(void) {
    if (uart_rx_q == NULL) {
        uart_rx_q = osMessageQueueNew(UART_RX_BUF_LENGTH, sizeof(uint8_t), NULL);
    }
    if (uart_tx_mutex == NULL) {
        uart_tx_mutex = osMutexNew(NULL);
    }

    // 인터럽트 수신 시작
    HAL_UART_Receive_IT(&huart3, &rx_data, 1);
    return true;
}

// 수신 인터럽트 콜백: 받은 데이터를 큐에 넣음
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART3) {
        // 큐에 데이터 넣기
        osMessageQueuePut(uart_rx_q, &rx_data, 0, 0);
        
        // [중요] 인터럽트 재활성화 (이게 없으면 딱 한 글자만 입력되고 멈춤)
        HAL_UART_Receive_IT(huart, &rx_data, 1);
    }
}

// 큐에 쌓인 데이터 개수 확인
uint32_t uartAvailable(uint8_t ch) {
    return (uart_rx_q != NULL) ? osMessageQueueGetCount(uart_rx_q) : 0;
}

// Non-blocking 읽기
uint8_t uartRead(uint8_t ch) {
    uint8_t ret = 0;
    if (uart_rx_q != NULL) {
        osMessageQueueGet(uart_rx_q, &ret, NULL, 0);
    }
    return ret;
}

// Blocking 읽기 (CLI에서 사용)
bool uartReadBlock(uint8_t ch, uint8_t *p_data, uint32_t timeout) {
    if (uart_rx_q != NULL) {
        return (osMessageQueueGet(uart_rx_q, p_data, NULL, timeout) == osOK);
    }
    return false;
}

// Mutex를 이용한 안전한 송신
uint32_t uartWrite(uint8_t ch, uint8_t *p_data, uint32_t len) {
    if (uart_tx_mutex == NULL) return 0;

    osMutexAcquire(uart_tx_mutex, osWaitForever);
    if (HAL_UART_Transmit(&huart3, p_data, len, 1000) != HAL_OK) {
        len = 0;
    }
    osMutexRelease(uart_tx_mutex);
    return len;
}

uint32_t uartPrintf(uint8_t ch, const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return uartWrite(ch, (uint8_t *)buf, len);
}