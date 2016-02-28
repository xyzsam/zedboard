#include <stdlib.h>
#include "xaxidma.h"
#include "xscugic.h"
#include "interrupts.h"

int g_dma_err;
int g_mm2s_done;
int g_s2mm_done;

int init_intc(XScuGic* int_device, XAxiDma* dma_device,
              int int_device_id, int s2mm_intr_id, int mm2s_intr_id) {
  XScuGic_Config* cfg;
  int status = 0;

  cfg = XScuGic_LookupConfig(int_device_id);
  if (!cfg) {
    xil_printf("No hardware config found for interrupt controller id %d\r\n", int_device_id);
    return -1;
  }

  status = XScuGic_CfgInitialize(int_device, cfg, cfg->CpuBaseAddress);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize interrupt controller with status %d\r\n", status);
    return -1;
  }

  // Set interrupt priorities and trigger type
  XScuGic_SetPriorityTriggerType(int_device, s2mm_intr_id, 0xA0, 0x3);
  XScuGic_SetPriorityTriggerType(int_device, mm2s_intr_id, 0xA8, 0x3);

  // Connect handlers
  status = XScuGic_Connect(int_device, s2mm_intr_id, (Xil_InterruptHandler)s2mm_isr, dma_device);
  if (status != XST_SUCCESS)
  {
    xil_printf("ERROR! Failed to connect s2mm_isr to the interrupt controller.\r\n", status);
    return -1;
  }
  status = XScuGic_Connect(int_device, mm2s_intr_id, (Xil_InterruptHandler)mm2s_isr, dma_device);
  if (status != XST_SUCCESS)
  {
    xil_printf("ERROR! Failed to connect mm2s_isr to the interrupt controller.\r\n", status);
    return -1;
  }

  // Enable all interrupts
  XScuGic_Enable(int_device, s2mm_intr_id);
  XScuGic_Enable(int_device, mm2s_intr_id);

  // Initialize exception table and register the interrupt controller handler with exception table
  Xil_ExceptionInit();
  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, int_device);

  // Enable non-critical exceptions
  Xil_ExceptionEnable();

  return 0;
}

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

