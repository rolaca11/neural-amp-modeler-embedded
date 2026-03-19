#ifndef FIR_FILTER_H
#define FIR_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "arm_math.h"

#define FIR_FILTER_MAX_TAPS       64
#define FIR_FILTER_MAX_BLOCK_SIZE 256

typedef enum
{
  FIR_OK        = 0x00,
  FIR_ERROR     = 0x01,
  FIR_BAD_PARAM = 0x02,
} FIR_StatusTypeDef;

typedef struct
{
  arm_fir_instance_f32 instance;
  float32_t            coeffs[FIR_FILTER_MAX_TAPS];
  float32_t            state[FIR_FILTER_MAX_TAPS + FIR_FILTER_MAX_BLOCK_SIZE - 1];
} FIR_HandleTypeDef;

FIR_StatusTypeDef FIR_Init(FIR_HandleTypeDef *hfir, uint16_t numTaps,
                           const float32_t *pCoeffs, uint32_t blockSize);

FIR_StatusTypeDef FIR_Process(FIR_HandleTypeDef *hfir,
                              const float32_t *pSrc, float32_t *pDst,
                              uint32_t blockSize);

FIR_StatusTypeDef FIR_Reset(FIR_HandleTypeDef *hfir);

#ifdef __cplusplus
}
#endif

#endif /* FIR_FILTER_H */
