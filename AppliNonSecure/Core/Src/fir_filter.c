#include "fir_filter.h"
#include <string.h>

FIR_StatusTypeDef FIR_Init(FIR_HandleTypeDef *hfir, uint16_t numTaps,
                           const float32_t *pCoeffs, uint32_t blockSize)
{
  if (hfir == NULL || pCoeffs == NULL)
    return FIR_BAD_PARAM;
  if (numTaps == 0 || numTaps > FIR_FILTER_MAX_TAPS)
    return FIR_BAD_PARAM;
  if (blockSize == 0 || blockSize > FIR_FILTER_MAX_BLOCK_SIZE)
    return FIR_BAD_PARAM;

  memcpy(hfir->coeffs, pCoeffs, numTaps * sizeof(float32_t));
  arm_fir_init_f32(&hfir->instance, numTaps, hfir->coeffs, hfir->state, blockSize);

  return FIR_OK;
}

FIR_StatusTypeDef FIR_Process(FIR_HandleTypeDef *hfir,
                              const float32_t *pSrc, float32_t *pDst,
                              uint32_t blockSize)
{
  if (hfir == NULL || pSrc == NULL || pDst == NULL)
    return FIR_BAD_PARAM;
  if (blockSize == 0 || blockSize > FIR_FILTER_MAX_BLOCK_SIZE)
    return FIR_BAD_PARAM;

  arm_fir_f32(&hfir->instance, pSrc, pDst, blockSize);

  return FIR_OK;
}

FIR_StatusTypeDef FIR_Reset(FIR_HandleTypeDef *hfir)
{
  if (hfir == NULL)
    return FIR_BAD_PARAM;

  memset(hfir->state, 0,
         (hfir->instance.numTaps + FIR_FILTER_MAX_BLOCK_SIZE - 1) * sizeof(float32_t));

  return FIR_OK;
}
