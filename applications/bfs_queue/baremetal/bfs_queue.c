/*
 * Test bench for BFS queue.
 */

#include <stdio.h>
#include "platform.h"
#include "xaxidma.h"
#include "xbfs_queue.h"
#include "xparameters.h"

#include "arm_timing_utils.h"
#include "dma_utils.h"
#include "generate.h"
#include "interrupts.h"
#include "lfsr.h"
#include "profiling.h"

#define ROWS 130
#define COLS 66
#define F_SIZE 9

static XBfs_queue bfs_queue_device;
static XAxiDma dma_device;

int init_bfs_queue(XBfs_queue *bfs_queue_device, int bfs_queue_device_id) {
  int status = 0;
  status = XBfs_queue_Initialize(bfs_queue_device, bfs_queue_device_id);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize device with error %d\r\n", status);
    return -1;
  }
  return 0;
}

void init_args(struct bench_args_t *args) {
  generate_binary(args);
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
  int status, i, j, num_failures;
  struct DmaChannel channel;
  struct DmaPacket packet;
  unsigned cycle_start, cycle_end;
  unsigned all_start, all_end;
  unsigned counter;
  struct bench_args_t *args;
  int *output_array;
  int correct_answer[N_LEVELS];
  counter = 0;
  init_platform();
  init_perfcounters(1, 0);

  InitPacket(&packet, &channel);

  // printf("Initializing DMA device\r\n");
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
  init_intc(&int_device, &dma_device, XPAR_PS7_SCUGIC_0_DEVICE_ID,
            XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,
            XPAR_FABRIC_AXI_DMA_0_MM2S_INTROUT_INTR);
#endif

  // printf("Initializing bfs queue.\r\n");
  cycle_start = get_cyclecount();
  init_bfs_queue(&bfs_queue_device, XPAR_XBFS_QUEUE_0_DEVICE_ID);
  XBfs_queue_Start(&bfs_queue_device);
  cycle_end = get_cyclecount();
  accel_init = cycle_end - cycle_start;

  // printf("Initializing input and output arrays\r\n");
  cycle_start = get_cyclecount();
  memset((void *)packet.TxBuf, 0, RX_BUFFER_BASE - TX_BUFFER_BASE);
  memset((void *)packet.RxBuf, TEST_RX_INVALID_VALUE,
         RX_BUFFER_HIGH - RX_BUFFER_BASE + 1);
  args = (struct bench_args_t *)packet.TxBuf;
  output_array = (int *)packet.RxBuf;
  init_args(args);
  packet.TxNumBytes = IN_STREAM_LEN * sizeof(uint32_t);
  packet.RxNumBytes = N_LEVELS * sizeof(uint32_t);
  cycle_end = get_cyclecount();
  prepare_data = cycle_end - cycle_start;

  // Prepare and measure DMA invocation latency.
  printf("Preparing packet.\r\n");
  cycle_start = get_cyclecount();
  status = PreparePacket(&packet);
  CHECK_STATUS_AND_QUIT(status, "preparing packet");
  status = SendPacket(&packet);
  cycle_end = get_cyclecount();
  dma_invocation = cycle_end - cycle_start;
  CHECK_STATUS_AND_QUIT(status, "sending packet");

  // printf("Waiting for accelerator to finish.\r\n");
  cycle_start = get_cyclecount();
  while (!XBfs_queue_IsDone(&bfs_queue_device)) {
    counter++;
  }
  cycle_end = get_cyclecount();
  accel_runtime = cycle_end - cycle_start;

  cycle_start = get_cyclecount();
  num_failures = 0;
  correct_answer[0] = 1;
  correct_answer[1] = 28;
  correct_answer[2] = 74;
  correct_answer[3] = 3;
  correct_answer[4] = 0;
  correct_answer[5] = 0;
  correct_answer[6] = 0;
  correct_answer[7] = 0;
  correct_answer[8] = 0;
  correct_answer[9] = 0;

  for (i = 0; i < N_LEVELS; i++) {
    if (output_array[i] != correct_answer[i]) {
      num_failures++;
      printf("Expected level %d = %d, got %d (0x%x).\r\n", i, correct_answer[i],
             output_array[i], output_array[i]);
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
  xil_printf("-RX preparing %d BDs: %d\r\n", packet.NumRxBds, rx_prepare);
  xil_printf("-TX preparing %d BDs: %d\r\n", packet.NumTxBds, tx_prepare);
  xil_printf("Prepare data: %d\r\n", prepare_data);
  xil_printf("BFS init: %d\r\n", accel_init);
  xil_printf("BFS runtime: %d, spun %d times\r\n", accel_runtime, counter);
  xil_printf("Output check: %d\r\n", check_time);
  xil_printf("Total runtime: %d\r\n", total_runtime);

  if (num_failures > 0)
    xil_printf("Failed %d times!\r\n", num_failures);
  else
    xil_printf("Test passed!\r\n");

  cleanup_platform();
  return 0;
}
