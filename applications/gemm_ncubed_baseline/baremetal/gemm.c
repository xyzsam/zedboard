/*
 * Test bench for GEMM, ncubed, baseline.
 */

#include <stdio.h>
#include "platform.h"
#include "xaxidma.h"
#include "xgemm.h"
#include "xparameters.h"

#include "dma_utils.h"
#include "interrupts.h"
#include "profiling.h"

#define ROWS 64

static XGemm gemm_device;
static XAxiDma dma_device;

int init_dma(XAxiDma *dma_device, struct DmaChannel *channel,
             int dma_device_id) {
  int status = 0;
  XAxiDma_Config *config;
  config = XAxiDma_LookupConfig(dma_device_id);
  if (!config) {
    printf("Failed to lookup DMA config.\r\n");
    return -1;
  }
  status = XAxiDma_CfgInitialize(dma_device, config);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize device with error %d\r\n", status);
    return -1;
  }

  XAxiDma_Reset(dma_device);
  while (!XAxiDma_ResetIsDone(dma_device)) {
  }
  XAxiDma_IntrEnable(dma_device,
                     (XAXIDMA_IRQ_IOC_MASK | XAXIDMA_IRQ_ERROR_MASK),
                     XAXIDMA_DMA_TO_DEVICE);
  XAxiDma_IntrEnable(dma_device,
                     (XAXIDMA_IRQ_IOC_MASK | XAXIDMA_IRQ_ERROR_MASK),
                     XAXIDMA_DEVICE_TO_DMA);

  g_dma_err = 0;
  g_mm2s_done = 0;
  g_s2mm_done = 0;
  return 0;
}

int init_gemm(XGemm *gemm_device, int gemm_device_id) {
  int status = 0;
  status = XGemm_Initialize(gemm_device, gemm_device_id);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize device with error %d\r\n", status);
    return -1;
  }
  return 0;
}

int init_intc(XScuGic *int_device, XAxiDma *dma_device, int int_device_id,
              int s2mm_intr_id, int mm2s_intr_id) {
  XScuGic_Config *cfg;
  int status = 0;

  cfg = XScuGic_LookupConfig(int_device_id);
  if (!cfg) {
    xil_printf("No hardware config found for interrupt controller id %d\r\n",
               int_device_id);
    return -1;
  }

  status = XScuGic_CfgInitialize(int_device, cfg, cfg->CpuBaseAddress);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize interrupt controller with status %d\r\n",
               status);
    return -1;
  }

  // Set interrupt priorities and trigger type
  XScuGic_SetPriorityTriggerType(int_device, s2mm_intr_id, 0xA0, 0x3);
  XScuGic_SetPriorityTriggerType(int_device, mm2s_intr_id, 0xA8, 0x3);

  // Connect handlers
  status = XScuGic_Connect(int_device, s2mm_intr_id,
                           (Xil_InterruptHandler)s2mm_isr, dma_device);
  if (status != XST_SUCCESS) {
    xil_printf(
        "ERROR! Failed to connect s2mm_isr to the interrupt controller.\r\n",
        status);
    return -1;
  }
  status = XScuGic_Connect(int_device, mm2s_intr_id,
                           (Xil_InterruptHandler)mm2s_isr, dma_device);
  if (status != XST_SUCCESS) {
    xil_printf(
        "ERROR! Failed to connect mm2s_isr to the interrupt controller.\r\n",
        status);
    return -1;
  }

  // Enable all interrupts
  XScuGic_Enable(int_device, s2mm_intr_id);
  XScuGic_Enable(int_device, mm2s_intr_id);

  // Initialize exception table and register the interrupt controller handler
  // with exception table
  Xil_ExceptionInit();
  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                               (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                               int_device);

  // Enable non-critical exceptions
  Xil_ExceptionEnable();

  return 0;
}

void init_packet(struct DmaPacket *packet, struct DmaChannel *channel) {
  packet->TxBuf = (u8 *)TX_BUFFER_BASE;
  packet->RxBuf = (u8 *)RX_BUFFER_BASE;
  packet->channel = channel;
}

void init_arrays(int *in, int *out, size_t rows) {
  int i, j;
  // Set the first array to a constant value.
  for (i = 0; i < rows; i++) {
    for (j = 0; j < rows; j++) {
      in[i * rows + j] = 3;
    }
  }

  // Set the second array to be the identity matrix.
  memset(&in[rows * rows], 0, rows * rows * sizeof(int));
  for (i = rows; i < 2 * rows; i++) {
    in[i * rows + i] = 1;
  }

  // Set the output array to be all -1.
  memset(out, -1, rows * rows * sizeof(int));
}

void print_buffer(int *in, size_t row_size) {
  int i, j;
  for (i = 0; i < row_size; i++) {
    for (j = 0; j < row_size; j++) {
      xil_printf("%d ", in[i * row_size + j]);
    }
    xil_printf("\n");
  }
}

int main() {
  int status, i, num_failures, ProcessedBdCount;
  struct DmaChannel channel;
  struct DmaPacket packet;
  size_t num_bytes = ROWS * ROWS * sizeof(int);
  unsigned cycle_start, cycle_end;
  unsigned all_start, all_end;
  unsigned counter;
  unsigned int *input_array;
  unsigned int *output_array;
  counter = 0;
  init_platform();
  init_perfcounters(1, 0);

  init_packet(&packet, &channel);

  cycle_start = get_cyclecount();
  all_start = cycle_start;
  init_dma(&dma_device, &channel, XPAR_AXIDMA_0_DEVICE_ID);
  status = TxSetup(&dma_device, &channel);
  CHECK_STATUS_AND_QUIT(status, "tx setup");
  status = RxSetup(&dma_device, &packet);
  CHECK_STATUS_AND_QUIT(status, "rx setup");
  cycle_end = get_cyclecount();
  dma_setup = cycle_end - cycle_start;

#ifdef ENABLE_INTERRUPTS
  printf("Setting up interrupts\r\n");
  init_intc(&int_device, &dma_device, XPAR_PS7_SCUGIC_0_DEVICE_ID,
            XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,
            XPAR_FABRIC_AXI_DMA_0_MM2S_INTROUT_INTR);
#endif

  cycle_start = get_cyclecount();
  init_gemm(&gemm_device, XPAR_XGEMM_0_DEVICE_ID);
  XGemm_Start(&gemm_device);
  cycle_end = get_cyclecount();
  gemm_init = cycle_end - cycle_start;

  // Initialize input and output arrays.
  cycle_start = get_cyclecount();
  memset((void *)packet.TxBuf, 0, RX_BUFFER_BASE - TX_BUFFER_BASE);
  memset((void *)packet.RxBuf, TEST_RX_INVALID_VALUE,
         RX_BUFFER_HIGH - RX_BUFFER_BASE + 1);
  input_array = (int *)packet.TxBuf;
  output_array = (int *)packet.RxBuf;
  init_arrays(input_array, output_array, ROWS);
  packet.TxNumBytes = 2 * num_bytes;
  packet.RxNumBytes = num_bytes;
  cycle_end = get_cyclecount();
  prepare_data = cycle_end - cycle_start;

  // Prepare and measure DMA invocation latency.
  cycle_start = get_cyclecount();
  status = PreparePacket(&packet);
  CHECK_STATUS_AND_QUIT(status, "preparing packet");
  status = SendPacket(&packet);
  cycle_end = get_cyclecount();
  dma_invocation = cycle_end - cycle_start;
  CHECK_STATUS_AND_QUIT(status, "sending packet");

  cycle_start = get_cyclecount();
  while (!XGemm_IsDone(&gemm_device)) {
    counter++;
  }
  cycle_end = get_cyclecount();
  gemm_runtime = cycle_end - cycle_start;

  cycle_start = get_cyclecount();
  num_failures = 0;
  for (i = 0; i < num_bytes; i++) {
    if (packet.TxBuf[i] != packet.RxBuf[i]) {
      num_failures++;
    }
  }
  cycle_end = get_cyclecount();
  check_time = cycle_end - cycle_start;

  all_end = cycle_end;
  total_runtime = all_end - all_start;

  xil_printf("DMA setup latency: %d\r\n", dma_setup);
  xil_printf("DMA invocation latency: %d\r\n", dma_invocation);
  xil_printf("-dcache flush: %d\r\n", dcache_flush);
  xil_printf("-dcache invalidate: %d\r\n", dcache_invalidate);
  xil_printf("-TX BD prepare: %d\r\n", tx_prepare);
  xil_printf("Prepare data: %d\r\n", prepare_data);
  xil_printf("GEMM init: %d\r\n", gemm_init);
  xil_printf("GEMM runtime: %d, spun %d times\r\n", gemm_runtime, counter);
  xil_printf("Output check: %d\r\n", check_time);
  xil_printf("Total runtime: %d\r\n", total_runtime);

  if (num_failures > 0)
    xil_printf("Failed %d times!\r\n", num_failures);
  else
    xil_printf("Test passed!\r\n");

  cleanup_platform();
  return 0;
}
