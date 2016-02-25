#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include "xaxidma.h"
#include "xscugic.h"

#define RESET_TIMEOUT_COUNTER 10000

extern int g_dma_err;
extern int g_mm2s_done;
extern int g_s2mm_done;

void s2mm_isr(void* CallbackRef);
void mm2s_isr(void* CallbackRef);

#endif
