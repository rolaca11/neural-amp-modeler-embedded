#ifndef __AUDIO_IO_H
#define __AUDIO_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "cmsis_os2.h"

#define AUDIO_IO_RX_BUF_SIZE    512
#define AUDIO_IO_MAX_SAMPLES    64

void audioIoInit(void);
void audioIoTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_IO_H */
