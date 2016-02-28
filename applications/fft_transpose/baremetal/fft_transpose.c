/*
 * FFT transpose testbench.
 */

#include <stdio.h>
#include "platform.h"
#include "xaxidma.h"
#include "xfft_transpose.h"
#include "xparameters.h"

#include "arm_timing_utils.h"
#include "dma_utils.h"
#include "interrupts.h"
#include "profiling.h"
#include "generate.h"

#define CORRECT_RESULT 777.71185
#define EPSILON 0.00001

// #define DEBUG

static XFft_transpose fft_transpose_device;
static XAxiDma dma_device;

int init_fft_transpose(XFft_transpose* fft_transpose_device, int fft_transpose_device_id) {
  int status = 0;
  status = XFft_transpose_Initialize(fft_transpose_device, fft_transpose_device_id);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize device with error %d\r\n", status);
    return -1;
  }
  return 0;
}

void init_arrays(float* in, float *out) {
	struct bench_args_t *args = (struct bench_args_t*) malloc(sizeof(struct bench_args_t));
	generate_data(args);
	memcpy(in, args, sizeof(struct bench_args_t));
	memset(out, 0, sizeof(struct bench_args_t));
	free(args);
}

int main()
{
  int status, i, num_failures;
  struct DmaChannel channel;
  struct DmaPacket packet;
  unsigned cycle_start, cycle_end;
  unsigned all_start, all_end;
  unsigned counter;
  float *input_array, *output_array;
  struct bench_args_t *fft_out;
  float accel_result_hash = 0;
  counter = 0;
  init_platform();
  init_perfcounters(1,0);

  InitPacket(&packet, &channel);

  cycle_start = get_cyclecount();
  all_start = cycle_start;
  InitDma(&dma_device, &channel, XPAR_AXIDMA_0_DEVICE_ID);
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

#ifdef DEBUG
  printf("Initializing MD-KNN.\r\n");
#endif
  cycle_start = get_cyclecount();
  init_fft_transpose(&fft_transpose_device, XPAR_XFFT_TRANSPOSE_0_DEVICE_ID);
  XFft_transpose_Start(&fft_transpose_device);
  cycle_end = get_cyclecount();
  accel_init = cycle_end - cycle_start;

#ifdef DEBUG
  printf("Initializing input and output arrays\r\n");
#endif
  cycle_start = get_cyclecount();
  memset((void*)packet.TxBuf, TEST_TX_INVALID_VALUE, RX_BUFFER_BASE - TX_BUFFER_BASE);
  memset((void*)packet.RxBuf, TEST_RX_INVALID_VALUE, RX_BUFFER_HIGH - RX_BUFFER_BASE + 1);
  input_array = (int*) packet.TxBuf;
  output_array = (int*) packet.RxBuf;
  init_arrays(input_array, output_array);
  packet.TxNumBytes = sizeof(struct bench_args_t);
  packet.RxNumBytes = sizeof(struct bench_args_t);
  cycle_end = get_cyclecount();
  prepare_data = cycle_end - cycle_start;

  // Prepare and measure DMA invocation latency.
#ifdef DEBUG
  printf("Preparing and sending packet.\r\n");
#endif
  cycle_start = get_cyclecount();
  status = PreparePacket(&packet);
  CHECK_STATUS_AND_QUIT(status, "preparing packet");
  status = SendPacket(&packet);
  cycle_end = get_cyclecount();
  dma_invocation = cycle_end - cycle_start;
  CHECK_STATUS_AND_QUIT(status, "sending packet");

#ifdef DEBUG
  printf("Waiting for device.\r\n");
#endif
  cycle_start = get_cyclecount();
  while (!XFft_transpose_IsDone(&fft_transpose_device)) { counter++; }
  cycle_end = get_cyclecount();
  accel_runtime = cycle_end - cycle_start;

  cycle_start = get_cyclecount();
  num_failures = 0;
  fft_out = (struct bench_args_t*) packet.RxBuf;
  for (i = 0; i < 512; i++)
	  accel_result_hash += fft_out->work_x[i];
  for (i = 0; i < 512; i++)
	  accel_result_hash += fft_out->work_y[i];

  if (accel_result_hash == accel_result_hash &&
	  abs(accel_result_hash - CORRECT_RESULT) > EPSILON) {
	  num_failures = 1;
  }
  cycle_end = get_cyclecount();
  check_time = cycle_end - cycle_start;

  all_end = cycle_end;
  total_runtime = all_end - all_start;

  if (num_failures == 0) {
	  printf("TEST PASSED!\r\n");
  } else {
	  printf("TEST FAILED!\r\n");
	  printf("Expected hash = %2.8f, got %2.8f instead.\r\n", CORRECT_RESULT, accel_result_hash);
  }
  printf("Accelerator result hash: %2.8f\r\n", accel_result_hash);
  xil_printf("DMA setup latency: %d\r\n", dma_setup);
  xil_printf("DMA invocation latency: %d\r\n", dma_invocation);
  xil_printf("-dcache flush: %d\r\n", dcache_flush);
  xil_printf("-dcache invalidate: %d\r\n", dcache_invalidate);
  xil_printf("-TX preparing %d BDs: %d\r\n", packet.NumTxBds, tx_prepare);
  xil_printf("-RX preparing %d BDs: %d\r\n", packet.NumRxBds, rx_prepare);
  xil_printf("Prepare data: %d\r\n", prepare_data);
  xil_printf("FFT-transpose init: %d\r\n", accel_init);
  xil_printf("FFT-transpose runtime: %d\r\n", accel_runtime);
  xil_printf("Output check: %d\r\n", check_time);
  xil_printf("Total runtime: %d\r\n", total_runtime);

  cleanup_platform();
  return 0;
}
