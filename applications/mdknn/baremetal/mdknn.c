/*
 * MD-KNN test bench.
 */

#include <stdio.h>
#include "platform.h"
#include "xaxidma.h"
#include "xmdknn.h"
#include "xparameters.h"

#include "arm_timing_utils.h"
#include "dma_utils.h"
#include "interrupts.h"
#include "profiling.h"
#include "generate.h"

#define CORRECT_RESULT 0.00064724
#define EPSILON 0.00001

// #define DEBUG

static XMdknn mdknn_device;
static XAxiDma dma_device;

int init_mdknn(XMdknn* mdknn_device, int mdknn_device_id) {
  int status = 0;
  status = XMdknn_Initialize(mdknn_device, mdknn_device_id);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize device with error %d\r\n", status);
    return -1;
  }
  return 0;
}

void init_arrays(float* in, float *out) {
  struct bench_args_float_t *args =
      (struct bench_args_float_t *)malloc(sizeof(struct bench_args_float_t));
        generate_data(args);
	memcpy(in, &args->position_x[0], sizeof(struct bench_args_float_t));
	memset(out, 0, sizeof(struct bench_args_float_t));
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
  init_mdknn(&mdknn_device, XPAR_XMDKNN_0_DEVICE_ID);
  XMdknn_Start(&mdknn_device);
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
  packet.TxNumBytes = (3*nAtoms + nAtoms*maxNeighbors)*sizeof(TYPE);
  packet.RxNumBytes = (3*nAtoms)*sizeof(TYPE);
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
  while (!XMdknn_IsDone(&mdknn_device)) { counter++; }
  cycle_end = get_cyclecount();
  accel_runtime = cycle_end - cycle_start;

  cycle_start = get_cyclecount();
  num_failures = 0;
  for (i = 0; i < 3*nAtoms; i++) {
	  accel_result_hash += output_array[i];
  }
  if (accel_result_hash == accel_result_hash &&  // Checks for NaN.
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
  xil_printf("MD-KNN init: %d\r\n", accel_init);
  xil_printf("MD-KNN runtime: %d\r\n", accel_runtime);
  xil_printf("Output check: %d\r\n", check_time);
  xil_printf("Total runtime: %d\r\n", total_runtime);

  cleanup_platform();
  return 0;
}
