#ifndef UART_IO_H
#define UART_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "cmsis_os2.h"
#include "arm_math.h"

#define UART_IO_RX_BUF_SIZE  512
#define UART_IO_MAX_SAMPLES  256

void     UartIo_Init(UART_HandleTypeDef *huart);
uint16_t UartIo_Receive(float32_t *samples, uint16_t maxSamples);
void     UartIo_SendSamples(const char *tag, const float32_t *samples, uint16_t count);

#ifdef __cplusplus
}
#endif

#endif /* UART_IO_H */
