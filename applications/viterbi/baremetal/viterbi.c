/*
 * Viterbi test bench.
 */

#include <stdio.h>
#include "platform.h"
#include "xaxidma.h"
#include "xviterbi.h"
#include "xparameters.h"

#include "arm_timing_utils.h"
#include "dma_utils.h"
#include "generate.h"
#include "interrupts.h"
#include "profiling.h"
#include "viterbi.h"

#define NUM_STATES 32
#define NUM_OBS 128

// This was the result obtained by running viterbi on a CPU, using the
// provided pseudorandom input.
#define CORRECT_RESULT 9

static XViterbi viterbi_device;
static XAxiDma dma_device;

int init_dma(XAxiDma* dma_device, struct DmaChannel * channel, int dma_device_id) {
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
  while (!XAxiDma_ResetIsDone(dma_device)) {}
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

int init_viterbi(XViterbi* viterbi_device, int viterbi_device_id) {
  int status = 0;
  status = XViterbi_Initialize(viterbi_device, viterbi_device_id);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize device with error %d\r\n", status);
    return -1;
  }
  return 0;
}

int init_intc(XScuGic* int_device, XAxiDma* dma_device, int int_device_id, int s2mm_intr_id, int mm2s_intr_id) {
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

void init_packet(struct DmaPacket* packet, struct DmaChannel *channel) {
	packet->TxBuf = (u8*) TX_BUFFER_BASE;
	packet->RxBuf = (u8*) RX_BUFFER_BASE;
	packet->channel = channel;
}

void init_arrays(int* in) {
  struct bench_args_float_t *args = (struct bench_args_float_t*) malloc(sizeof(struct bench_args_float_t));
  generate_binary(args);
  memcpy(in, args, sizeof(struct bench_args_float_t));
  free(args);
}

int main()
{
  int status;
  struct DmaChannel channel;
  struct DmaPacket packet;
  unsigned cycle_start, cycle_end;
  unsigned all_start, all_end;
  unsigned counter;
  unsigned int *input_array;
  int viterbi_result;
  counter = 0;
  init_platform();
  init_perfcounters(1,0);

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
  init_intc(&int_device,
            &dma_device,
            XPAR_PS7_SCUGIC_0_DEVICE_ID,
            XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,
            XPAR_FABRIC_AXI_DMA_0_MM2S_INTROUT_INTR);
#endif

  // printf("Initializing Viterbi.\r\n");
  cycle_start = get_cyclecount();
  init_viterbi(&viterbi_device, XPAR_XVITERBI_0_DEVICE_ID);
  XViterbi_Start(&viterbi_device);
  cycle_end = get_cyclecount();
  accel_init = cycle_end - cycle_start;

  // printf("Initializing input and output arrays\r\n");
  cycle_start = get_cyclecount();
  memset((void*)packet.TxBuf, TEST_TX_INVALID_VALUE, RX_BUFFER_BASE - TX_BUFFER_BASE);
  memset((void*)packet.RxBuf, TEST_RX_INVALID_VALUE, RX_BUFFER_HIGH - RX_BUFFER_BASE + 1);
  input_array = (int*) packet.TxBuf;
  init_arrays(input_array);
  packet.TxNumBytes = sizeof(struct bench_args_float_t);
  packet.RxNumBytes = 0;
  cycle_end = get_cyclecount();
  prepare_data = cycle_end - cycle_start;

  // Prepare and measure DMA invocation latency.
  // printf("Preparing and sending packet.\r\n");
  cycle_start = get_cyclecount();
  status = PreparePacket(&packet);
  CHECK_STATUS_AND_QUIT(status, "preparing packet");
  status = SendPacket(&packet);
  cycle_end = get_cyclecount();
  dma_invocation = cycle_end - cycle_start;
  CHECK_STATUS_AND_QUIT(status, "sending packet");

  // printf("Waiting for device.\r\n");
  cycle_start = get_cyclecount();
  while (!XViterbi_IsDone(&viterbi_device)) { counter++; }
  cycle_end = get_cyclecount();
  accel_runtime = cycle_end - cycle_start;
  viterbi_result = XViterbi_Get_return(&viterbi_device);

  cycle_start = get_cyclecount();
  if (viterbi_result != CORRECT_RESULT) {
	  printf("Result: got %d, expected %d.\r\n", viterbi_result, CORRECT_RESULT);
  } else {
	  xil_printf("Test passed!\r\n");
  }
  cycle_end = get_cyclecount();
  check_time = cycle_end - cycle_start;

  all_end = cycle_end;
  total_runtime = all_end - all_start;

  xil_printf("DMA setup latency: %d\r\n", dma_setup);
  xil_printf("DMA invocation latency: %d\r\n", dma_invocation);
  xil_printf("-dcache flush: %d\r\n", dcache_flush);
  xil_printf("-dcache invalidate: %d\r\n", dcache_invalidate);
  xil_printf("-TX preparing %d BDs: %d\r\n", packet.NumTxBds, tx_prepare);
  xil_printf("Prepare data: %d\r\n", prepare_data);
  xil_printf("Viterbi init: %d\r\n", accel_init);
  xil_printf("Viterbi runtime: %d, spun %d times\r\n", accel_runtime, counter);
  xil_printf("Output check: %d\r\n", check_time);
  xil_printf("Total runtime: %d\r\n", total_runtime);

  cleanup_platform();
  return 0;
}
