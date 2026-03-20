#include "uart_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TX_BUF_SIZE 2048

static UART_HandleTypeDef *pUart;
static uint8_t rx_buf[UART_IO_RX_BUF_SIZE];
static uint8_t parse_buf[UART_IO_RX_BUF_SIZE];
static uint8_t tx_buf[TX_BUF_SIZE];
static volatile uint16_t rx_len;
static osSemaphoreId_t rx_sem;

static int ftoa(char *buf, int bufLen, float val)
{
  int neg = 0;
  if (val < 0.0f) { neg = 1; val = -val; }

  uint32_t intPart = (uint32_t)val;
  uint32_t fracPart = (uint32_t)((val - (float)intPart) * 1000.0f + 0.5f);
  if (fracPart >= 1000) { intPart++; fracPart = 0; }

  int pos = 0;
  if (neg && pos < bufLen) buf[pos++] = '-';

  /* Integer part (reverse digits) */
  char tmp[12];
  int digits = 0;
  if (intPart == 0)
    tmp[digits++] = '0';
  else
    while (intPart > 0) { tmp[digits++] = '0' + (intPart % 10); intPart /= 10; }
  for (int i = digits - 1; i >= 0 && pos < bufLen; i--)
    buf[pos++] = tmp[i];

  if (pos < bufLen) buf[pos++] = '.';

  /* 3 decimal digits */
  if (pos < bufLen) buf[pos++] = '0' + (fracPart / 100);
  if (pos < bufLen) buf[pos++] = '0' + ((fracPart / 10) % 10);
  if (pos < bufLen) buf[pos++] = '0' + (fracPart % 10);

  return pos;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if (huart->Instance == pUart->Instance)
  {
    rx_len = Size;
    osSemaphoreRelease(rx_sem);
  }
}

static uint16_t parseCsvFloat(const uint8_t *buf, uint16_t len,
                              float32_t *samples, uint16_t maxSamples)
{
  uint16_t count = 0;
  const char *p = (const char *)buf;
  const char *end = p + len;

  while (p < end && count < maxSamples)
  {
    while (p < end && (*p == ' ' || *p == '\r' || *p == '\n'))
      p++;
    if (p >= end)
      break;

    char *endptr;
    float val = strtof(p, &endptr);
    if (endptr == p)
      break;

    samples[count++] = val;
    p = endptr;

    if (p < end && *p == ',')
      p++;
  }
  return count;
}

void UartIo_Init(UART_HandleTypeDef *huart)
{
  pUart = huart;

  rx_sem = osSemaphoreNew(1, 0, NULL);
  if (rx_sem == NULL)
  {
    printf("[UART] ERROR: Failed to create semaphore\r\n");
    return;
  }

  HAL_NVIC_SetPriority(LPUART1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(LPUART1_IRQn);

  if (HAL_UARTEx_ReceiveToIdle_IT(pUart, rx_buf, UART_IO_RX_BUF_SIZE) != HAL_OK)
  {
    printf("[UART] ERROR: Failed to start reception\r\n");
  }
}

uint16_t UartIo_Receive(float32_t *samples, uint16_t maxSamples)
{
  if (osSemaphoreAcquire(rx_sem, osWaitForever) != osOK)
    return 0;

  uint16_t len = rx_len;
  memcpy(parse_buf, rx_buf, len);

  /* Restart reception immediately */
  HAL_UARTEx_ReceiveToIdle_IT(pUart, rx_buf, UART_IO_RX_BUF_SIZE);

  return parseCsvFloat(parse_buf, len, samples, maxSamples);
}

void UartIo_SendSamples(const char *tag, const float32_t *samples, uint16_t count)
{
  int pos = 0;
  int remaining = TX_BUF_SIZE;

  /* Tag */
  int tagLen = (int)strlen(tag);
  if (tagLen > remaining) tagLen = remaining;
  memcpy(&tx_buf[pos], tag, tagLen);
  pos += tagLen;
  remaining -= tagLen;

  /* Samples as CSV */
  for (uint16_t i = 0; i < count && remaining > 6; i++)
  {
    if (i > 0 && remaining > 1)
    {
      tx_buf[pos++] = ',';
      remaining--;
    }
    int n = ftoa((char *)&tx_buf[pos], remaining, samples[i]);
    pos += n;
    remaining -= n;
  }

  /* CRLF */
  if (remaining >= 2)
  {
    tx_buf[pos++] = '\r';
    tx_buf[pos++] = '\n';
  }

  HAL_UART_Transmit(pUart, tx_buf, (uint16_t)pos, HAL_MAX_DELAY);
}
