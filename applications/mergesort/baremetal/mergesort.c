/*
 * Mergesort testbench.
 */

#include <stdio.h>
#include "platform.h"
#include "xaxidma.h"
#include "xmergesort.h"
#include "xparameters.h"

#include "arm_timing_utils.h"
#include "dma_utils.h"
#include "interrupts.h"
#include "profiling.h"

#define NUM 4096

static XMergesort mergesort_device;
static XAxiDma dma_device;

int init_mergesort(XMergesort* mergesort_device, int mergesort_device_id) {
  int status = 0;
  status = XMergesort_Initialize(mergesort_device, mergesort_device_id);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize device with error %d\r\n", status);
    return -1;
  }
  return 0;
}

void init_arrays(int* in, int *out) {
	int i;
	// Init a reverse-sorted array, just to make validation easier...
	for (i = 0; i < NUM; i++) {
		in[i] = NUM - i;
	}
	memset(out, 0, NUM*sizeof(int));
}

int main()
{
  int status, i, num_failures;
  struct DmaChannel channel;
  struct DmaPacket packet;
  unsigned cycle_start, cycle_end;
  unsigned all_start, all_end;
  unsigned counter;
  int *input_array, *output_array;
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

  printf("Initializing Viterbi.\r\n");
  cycle_start = get_cyclecount();
  init_mergesort(&mergesort_device, XPAR_XMERGESORT_0_DEVICE_ID);
  XMergesort_Start(&mergesort_device);
  cycle_end = get_cyclecount();
  accel_init = cycle_end - cycle_start;

  printf("Initializing input and output arrays\r\n");
  cycle_start = get_cyclecount();
  memset((void*)packet.TxBuf, TEST_TX_INVALID_VALUE, RX_BUFFER_BASE - TX_BUFFER_BASE);
  memset((void*)packet.RxBuf, TEST_RX_INVALID_VALUE, RX_BUFFER_HIGH - RX_BUFFER_BASE + 1);
  input_array = (int*) packet.TxBuf;
  output_array = (int*) packet.RxBuf;
  init_arrays(input_array, output_array);
  packet.TxNumBytes = NUM*sizeof(int);
  packet.RxNumBytes = NUM*sizeof(int);
  cycle_end = get_cyclecount();
  prepare_data = cycle_end - cycle_start;

  // Prepare and measure DMA invocation latency.
  printf("Preparing and sending packet.\r\n");
  cycle_start = get_cyclecount();
  status = PreparePacket(&packet);
  CHECK_STATUS_AND_QUIT(status, "preparing packet");
  status = SendPacket(&packet);
  cycle_end = get_cyclecount();
  dma_invocation = cycle_end - cycle_start;
  CHECK_STATUS_AND_QUIT(status, "sending packet");

  printf("Waiting for device.\r\n");
  cycle_start = get_cyclecount();
  while (!XMergesort_IsDone(&mergesort_device)) { counter++; }
  cycle_end = get_cyclecount();
  accel_runtime = cycle_end - cycle_start;

  cycle_start = get_cyclecount();
  num_failures = 0;
  for (i = 0; i < NUM; i++) {
	  if (output_array[i] != input_array[NUM-i-1]) {
		  num_failures++;
		  printf("At %d, expected %d, got %d.\r\n", i, input_array[NUM-i-1], output_array[i]);
	  }
  }
  cycle_end = get_cyclecount();
  check_time = cycle_end - cycle_start;

  all_end = cycle_end;
  total_runtime = all_end - all_start;

  if (num_failures == 0) {
	  printf("TEST PASSED!\r\n");
  } else {
	  printf("TEST FAILED!\r\n");
  }
  xil_printf("DMA setup latency: %d\r\n", dma_setup);
  xil_printf("DMA invocation latency: %d\r\n", dma_invocation);
  xil_printf("-dcache flush: %d\r\n", dcache_flush);
  xil_printf("-dcache invalidate: %d\r\n", dcache_invalidate);
  xil_printf("-TX preparing %d BDs: %d\r\n", packet.NumTxBds, tx_prepare);
  xil_printf("-RX preparing %d BDs: %d\r\n", packet.NumRxBds, rx_prepare);
  xil_printf("Prepare data: %d\r\n", prepare_data);
  xil_printf("Mergesort init: %d\r\n", accel_init);
  xil_printf("Mergesort runtime: %d, spun %d times\r\n", accel_runtime, counter);
  xil_printf("Output check: %d\r\n", check_time);
  xil_printf("Total runtime: %d\r\n", total_runtime);

  cleanup_platform();
  return 0;
}
