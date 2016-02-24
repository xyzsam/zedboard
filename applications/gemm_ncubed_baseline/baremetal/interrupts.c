#include <stdlib.h>
#include "xaxidma.h"
#include "xscugic.h"
#include "interrupts.h"

int g_dma_err;
int g_mm2s_done;
int g_s2mm_done;

// Private functions
void s2mm_isr(void* CallbackRef)
{

  // Local variables
  int      irq_status;
  int      time_out;
  XAxiDma* p_dma_inst = (XAxiDma*)CallbackRef;

  // Disable interrupts
  XAxiDma_IntrDisable(p_dma_inst, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);
  XAxiDma_IntrDisable(p_dma_inst, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

  // Read pending interrupts
  irq_status = XAxiDma_IntrGetIrq(p_dma_inst, XAXIDMA_DEVICE_TO_DMA);

  // Acknowledge pending interrupts
  XAxiDma_IntrAckIrq(p_dma_inst, irq_status, XAXIDMA_DEVICE_TO_DMA);

  // If no interrupt is asserted, we do not do anything
  if (!(irq_status & XAXIDMA_IRQ_ALL_MASK))
    return;

  // If error interrupt is asserted, raise error flag, reset the
  // hardware to recover from the error, and return with no further
  // processing.
  if ((irq_status & XAXIDMA_IRQ_ERROR_MASK))
  {

    g_dma_err = 1;

    // Reset should never fail for transmit channel
    XAxiDma_Reset(p_dma_inst);

    time_out = RESET_TIMEOUT_COUNTER;
    while (time_out)
    {
      if (XAxiDma_ResetIsDone(p_dma_inst))
        break;
      time_out -= 1;
    }

    return;
  }

  // Completion interrupt asserted
  if (irq_status & XAXIDMA_IRQ_IOC_MASK)
    g_s2mm_done = 1;

  // Re-enable interrupts
  XAxiDma_IntrEnable(p_dma_inst, (XAXIDMA_IRQ_IOC_MASK | XAXIDMA_IRQ_ERROR_MASK), XAXIDMA_DMA_TO_DEVICE);
  XAxiDma_IntrEnable(p_dma_inst, (XAXIDMA_IRQ_IOC_MASK | XAXIDMA_IRQ_ERROR_MASK), XAXIDMA_DEVICE_TO_DMA);

}

void mm2s_isr(void* CallbackRef)
{

  // Local variables
  int      irq_status;
  int      time_out;
  XAxiDma* p_dma_inst = (XAxiDma*)CallbackRef;

  // Disable interrupts
  XAxiDma_IntrDisable(p_dma_inst, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);
  XAxiDma_IntrDisable(p_dma_inst, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

  // Read pending interrupts
  irq_status = XAxiDma_IntrGetIrq(p_dma_inst, XAXIDMA_DMA_TO_DEVICE);

  // Acknowledge pending interrupts
  XAxiDma_IntrAckIrq(p_dma_inst, irq_status, XAXIDMA_DMA_TO_DEVICE);

  // If no interrupt is asserted, we do not do anything
  if (!(irq_status & XAXIDMA_IRQ_ALL_MASK))
    return;
  // If error interrupt is asserted, raise error flag, reset the
  // hardware to recover from the error, and return with no further
  // processing.
  if (irq_status & XAXIDMA_IRQ_ERROR_MASK)
  {

    g_dma_err = 1;

    // Reset could fail and hang
    XAxiDma_Reset(p_dma_inst);

    time_out = RESET_TIMEOUT_COUNTER;

    while (time_out)
    {
      if (XAxiDma_ResetIsDone(p_dma_inst))
        break;

      time_out -= 1;
    }

    return;
  }

  // If completion interrupt is asserted, then set RxDone flag
  if (irq_status & XAXIDMA_IRQ_IOC_MASK)
    g_mm2s_done = 1;

  // Re-enable interrupts
  XAxiDma_IntrEnable(p_dma_inst, (XAXIDMA_IRQ_IOC_MASK | XAXIDMA_IRQ_ERROR_MASK), XAXIDMA_DMA_TO_DEVICE);
  XAxiDma_IntrEnable(p_dma_inst, (XAXIDMA_IRQ_IOC_MASK | XAXIDMA_IRQ_ERROR_MASK), XAXIDMA_DEVICE_TO_DMA);

}

