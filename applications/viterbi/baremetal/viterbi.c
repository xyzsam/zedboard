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

int init_viterbi(XViterbi* viterbi_device, int viterbi_device_id) {
  int status = 0;
  status = XViterbi_Initialize(viterbi_device, viterbi_device_id);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize device with error %d\r\n", status);
    return -1;
  }
  return 0;
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
