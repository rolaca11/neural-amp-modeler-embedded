#include "audio_io.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef handle_GPDMA1_Channel14;

static uint8_t rx_buf[AUDIO_IO_RX_BUF_SIZE];
static uint8_t parse_buf[AUDIO_IO_RX_BUF_SIZE];
static volatile uint16_t rx_len;

static osSemaphoreId_t rx_sem;

static int32_t input_samples[AUDIO_IO_MAX_SAMPLES];
static int32_t output_samples[AUDIO_IO_MAX_SAMPLES];

/* Stub DSP: passthrough */
static void audioDspProcess(const int32_t *in, int32_t *out, uint16_t count)
{
  memcpy(out, in, count * sizeof(int32_t));
}

static uint16_t parseCsvInt32(const uint8_t *buf, uint16_t len,
                              int32_t *samples, uint16_t max_samples)
{
  uint16_t count = 0;
  const uint8_t *p = buf;
  const uint8_t *end = buf + len;

  while (p < end && count < max_samples)
  {
    while (p < end && (*p == ' ' || *p == '\r' || *p == '\n'))
      p++;
    if (p >= end)
      break;

    char *endptr;
    long val = strtol((const char *)p, &endptr, 10);
    if ((const uint8_t *)endptr == p)
      break;

    samples[count++] = (int32_t)val;
    p = (const uint8_t *)endptr;

    if (p < end && *p == ',')
      p++;
  }

  return count;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if (huart->Instance == LPUART1)
  {
    rx_len = Size;
    osSemaphoreRelease(rx_sem);

    /* Restart reception */
    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buf, AUDIO_IO_RX_BUF_SIZE) != HAL_OK)
    {
      printf("[Audio] ERROR: Failed to restart DMA reception\r\n");
    }
  }
}

void audioIoInit(void)
{
}

void audioIoTask(void *argument)
{
  (void)argument;

  rx_sem = osSemaphoreNew(1, 0, NULL);
  if (rx_sem == NULL)
  {
    printf("[Audio] ERROR: Failed to create rx semaphore\r\n");
    return;
  }

  /* Enable interrupts from task context (after scheduler is running) */
  HAL_NVIC_SetPriority(GPDMA1_Channel14_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(GPDMA1_Channel14_IRQn);
  HAL_NVIC_SetPriority(LPUART1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(LPUART1_IRQn);

  /* Start first DMA reception (idle-line terminated) */
  if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buf, AUDIO_IO_RX_BUF_SIZE) != HAL_OK)
  {
    printf("[Audio] ERROR: Failed to start DMA reception\r\n");
  }
  printf("[Audio] Audio I/O task started, DMA reception active\r\n");

  for (;;)
  {
    if (osSemaphoreAcquire(rx_sem, osWaitForever) == osOK)
    {
      uint16_t len = rx_len;
      memcpy(parse_buf, rx_buf, len);

      uint16_t sampleCount = parseCsvInt32(parse_buf, len,
                                           input_samples, AUDIO_IO_MAX_SAMPLES);

      if (sampleCount > 0)
      {
        printf("[Audio] Received %u samples, processing\r\n", sampleCount);
        audioDspProcess(input_samples, output_samples, sampleCount);

        /* TODO: do something with output_samples (e.g. transmit back, feed DAC) */
      }
      else
      {
        printf("[Audio] Received %u bytes but parsed 0 samples\r\n", len);
      }
    }
  }
}
