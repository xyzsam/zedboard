/*
 * Needwun testbench.
 */

#include <stdio.h>
#include "platform.h"
#include "xaxidma.h"
#include "xneedwun.h"
#include "xparameters.h"

#include "arm_timing_utils.h"
#include "dma_utils.h"
#include "interrupts.h"
#include "profiling.h"
#include "generate.h"
#include "nw.h"

// #define DEBUG

static XNeedwun needwun_device;
static XAxiDma dma_device;

int init_needwun(XNeedwun* needwun_device, int needwun_device_id) {
  int status = 0;
  status = XNeedwun_Initialize(needwun_device, needwun_device_id);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize device with error %d\r\n", status);
    return -1;
  }
  return 0;
}

void init_arrays(u8* input, u8* output) {
	struct bench_args_t *args = (struct bench_args_t*) malloc(sizeof(struct bench_args_t));
	generate_data(args);
	memcpy(input, args, sizeof(struct bench_args_t));
	// memset(output, 0, sizeof(struct bench_args_t));
	free(args);
}

int check_result(char* out) {
  char *correct_alignedA =
      "cggccgcttag-tgggtgcggtgctaagggggctagagggcttg-tc-gcggggcacgggacatgcg--"
      "gcg-t--cgtaaaccaaacat-g-gcgccgggag-attatgctcttgcacg-acag-ta----g-gat-"
      "aaagc---agc-t___________________________________________________________"
      "______________________________________________";
  char *correct_alignedB = "--------tagct-ggtaccgt-ctaa-gtggc--ccggg-"
                           "ttgagcggctgggca--gg-c-tg-gaag-gttagcgt-"
                           "aaggagatatagtccg-cgggtgcagggtg-"
                           "gctggcccgtacagctacctggcgctgtgcgcgggagctt___________"
                           "___________________________________________________"
                           "___________________________________________";
  char *test_alignedA = out;
        char* test_alignedB = &out[ALEN+BLEN];

	int i, num_failures;
	num_failures = 0;
	for (i = 0; i < ALEN + BLEN; i++) {
		if (correct_alignedA[i] != test_alignedA[i]) {
			num_failures++;
			printf("In A[%d], expected %c, got %c.\r\n", i, correct_alignedA[i], test_alignedA[i]);
		}
		if (correct_alignedB[i] != test_alignedB[i]) {
			num_failures++;
			printf("In B[%d], expected %c, got %c.\r\n", i, correct_alignedB[i], test_alignedB[i]);
		}
	}
	return num_failures;
}

int main()
{
  int status, num_failures;
  struct DmaChannel channel;
  struct DmaPacket packet;
  unsigned cycle_start, cycle_end;
  unsigned all_start, all_end;
  unsigned counter;
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
  printf("Initializing Needleman Wunsch.\r\n");
#endif
  cycle_start = get_cyclecount();
  init_needwun(&needwun_device, XPAR_XNEEDWUN_0_DEVICE_ID);
  XNeedwun_Start(&needwun_device);
  cycle_end = get_cyclecount();
  accel_init = cycle_end - cycle_start;

#ifdef DEBUG
  printf("Initializing input and output arrays\r\n");
#endif
  cycle_start = get_cyclecount();
  memset((void*)packet.TxBuf, TEST_TX_INVALID_VALUE, RX_BUFFER_BASE - TX_BUFFER_BASE);
  memset((void*)packet.RxBuf, TEST_RX_INVALID_VALUE, RX_BUFFER_HIGH - RX_BUFFER_BASE + 1);
  init_arrays(packet.TxBuf, packet.RxBuf);
  packet.TxNumBytes = 2*(ALEN + BLEN);
  packet.RxNumBytes = 2*(ALEN + BLEN);
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
  while (!XNeedwun_IsDone(&needwun_device)) { counter++; }
  cycle_end = get_cyclecount();
  accel_runtime = cycle_end - cycle_start;

  cycle_start = get_cyclecount();
  num_failures = check_result((char*)packet.RxBuf);
  cycle_end = get_cyclecount();
  check_time = cycle_end - cycle_start;

  all_end = cycle_end;
  total_runtime = all_end - all_start;

  if (num_failures == 0) {
	  printf("TEST PASSED!\r\n");
  } else {
	  printf("TEST FAILED!\r\n");
	  printf("Got %d mismatching pairs.\r\n", num_failures);
  }
  xil_printf("DMA setup latency: %d\r\n", dma_setup);
  xil_printf("DMA invocation latency: %d\r\n", dma_invocation);
  xil_printf("-dcache flush: %d\r\n", dcache_flush);
  xil_printf("-dcache invalidate: %d\r\n", dcache_invalidate);
  xil_printf("-TX preparing %d BDs: %d\r\n", packet.NumTxBds, tx_prepare);
  xil_printf("-RX preparing %d BDs: %d\r\n", packet.NumRxBds, rx_prepare);
  xil_printf("Prepare data: %d\r\n", prepare_data);
  xil_printf("Needwun init: %d\r\n", accel_init);
  xil_printf("Needwun runtime: %d\r\n", accel_runtime);
  xil_printf("Output check: %d\r\n", check_time);
  xil_printf("Total runtime: %d\r\n", total_runtime);

  cleanup_platform();
  return 0;
}
