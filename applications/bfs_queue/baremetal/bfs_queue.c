/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
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

int init_bfs_queue(XBfs_queue* bfs_queue_device, int bfs_queue_device_id) {
  int status = 0;
  status = XBfs_queue_Initialize(bfs_queue_device, bfs_queue_device_id);
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

void init_args(struct bench_args_t* args) {
  generate_binary(args);
}

void print_buffer(int* in, size_t row_size) {
  int i, j;
  for (i = 0; i < row_size; i++) {
    for (j = 0; j < row_size; j++) {
      xil_printf("%d ", in[i*row_size + j]);
    }
    xil_printf("\n");
  }
}

int main()
{
  int status, i, j, num_failures;
  struct DmaChannel channel;
  struct DmaPacket packet;
  unsigned cycle_start, cycle_end;
  unsigned all_start, all_end;
  unsigned counter;
  struct bench_args_t *args;
  int* output_array;
  int correct_answer[N_LEVELS];
  counter = 0;
  init_platform();
  init_perfcounters(1,0);

  init_packet(&packet, &channel);

  printf("Initializing DMA device\r\n");
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

  cycle_start = get_cyclecount();
  printf("Initializing bfs queue.\r\n");
  init_bfs_queue(&bfs_queue_device, XPAR_XBFS_QUEUE_0_DEVICE_ID);
  XBfs_queue_Start(&bfs_queue_device);
  cycle_end = get_cyclecount();
  bfs_queue_init = cycle_end - cycle_start;

  printf("Initializing input and output arrays\r\n");
  cycle_start = get_cyclecount();
  memset((void*)packet.TxBuf, 0, RX_BUFFER_BASE - TX_BUFFER_BASE);
  memset((void*)packet.RxBuf, TEST_RX_INVALID_VALUE, RX_BUFFER_HIGH - RX_BUFFER_BASE + 1);
  args = (struct bench_args_t*) packet.TxBuf;
  output_array = (int*) packet.RxBuf;
  init_args(args);
  packet.TxNumBytes = IN_STREAM_LEN*sizeof(uint32_t);
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

  printf("Waiting for accelerator to finish.\r\n");
  cycle_start = get_cyclecount();
  while (!XBfs_queue_IsDone(&bfs_queue_device)) { counter++; }
  cycle_end = get_cyclecount();
  bfs_queue_runtime = cycle_end - cycle_start;

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
		printf("Expected level %d = %d, got %d (0x%x).\r\n", i, correct_answer[i], output_array[i], output_array[i]);
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
  xil_printf("BFS init: %d\r\n", bfs_queue_init);
  xil_printf("BFS runtime: %d, spun %d times\r\n", bfs_queue_runtime, counter);
  xil_printf("Output check: %d\r\n", check_time);
  xil_printf("Total runtime: %d\r\n", total_runtime);


  if (num_failures > 0)
	  xil_printf("Failed %d times!\r\n", num_failures);
  else
	  xil_printf("Test passed!\r\n");

  cleanup_platform();
  return 0;
}
