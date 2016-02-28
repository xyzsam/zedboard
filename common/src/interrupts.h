#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include "xaxidma.h"
#include "xscugic.h"

#define RESET_TIMEOUT_COUNTER 10000

extern int g_dma_err;
extern int g_mm2s_done;
extern int g_s2mm_done;

int init_intc(XScuGic* int_device, XAxiDma* dma_device,
              int int_device_id, int s2mm_intr_id, int mm2s_intr_id);
void s2mm_isr(void* CallbackRef);
void mm2s_isr(void* CallbackRef);

#endif
