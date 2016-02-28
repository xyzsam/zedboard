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
#include "xstencil.h"
#include "xparameters.h"

#include "arm_timing_utils.h"
#include "dma_utils.h"
#include "interrupts.h"
#include "profiling.h"

#define ROWS 130
#define COLS 66
#define F_SIZE 9

static XStencil stencil_device;
static XAxiDma dma_device;

int init_stencil(XStencil* stencil_device, int stencil_device_id) {
  int status = 0;
  status = XStencil_Initialize(stencil_device, stencil_device_id);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize device with error %d\r\n", status);
    return -1;
  }
  return 0;
}

void init_arrays(int* in, int* out, size_t rows, size_t cols, size_t filt_size) {
  int i, j;
  // Set the input array to a constant value.
  for (i = 0; i < rows; i++) {
	  for (j = 0; j < cols; j++) {
		  in[i*cols + j] = 3;
	  }
  }

  // Set the filter to be zeros with a center of 1 (thus the identity operator).
  for (i = 0; i < filt_size; i++) {
	  if (i == (filt_size-1)/2)
		  in[rows*cols+i] = 1;
	  else
		  in[rows*cols+i] = 0;
  }

  // Set the output array to be all -1.
  memset(out, -1, rows*cols*sizeof(int));
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
  size_t num_bytes = ROWS*COLS*sizeof(int);
  unsigned cycle_start, cycle_end;
  unsigned all_start, all_end;
  unsigned counter;
  unsigned int *input_array;
  unsigned int *output_array;
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

  cycle_start = get_cyclecount();
  //printf("Initializing Stencil.\r\n");
  init_stencil(&stencil_device, XPAR_XSTENCIL_0_DEVICE_ID);
  XStencil_Start(&stencil_device);
  cycle_end = get_cyclecount();
  accel_init = cycle_end - cycle_start;

  //printf("Initializing input and output arrays\r\n");
  cycle_start = get_cyclecount();
  memset((void*)packet.TxBuf, 0, RX_BUFFER_BASE - TX_BUFFER_BASE);
  memset((void*)packet.RxBuf, TEST_RX_INVALID_VALUE, RX_BUFFER_HIGH - RX_BUFFER_BASE + 1);
  input_array = (int*) packet.TxBuf;
  output_array = (int*) packet.RxBuf;
  init_arrays(input_array, output_array, ROWS, COLS, F_SIZE);
  packet.TxNumBytes = num_bytes+F_SIZE*sizeof(int);
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
  while (!XStencil_IsDone(&stencil_device)) { counter++; }
  cycle_end = get_cyclecount();
  accel_runtime = cycle_end - cycle_start;

  cycle_start = get_cyclecount();
  num_failures = 0;
  for (i = 0; i < ROWS; i++) {
	  for (j = 0; j < COLS; j++) {
		  if (i < ROWS-2 && j < COLS-2) {
			  if (output_array[i*COLS + j] != 3) {
				  num_failures++;
				  // if (output_array[i*COLS + j] != TEST_RX_INVALID_32)
					//  xil_printf("At (%d, %d), expected 0x%x\t, Rx: 0x%x\r\n", i, j, 3, output_array[i*COLS+j]);
			  }

		  } else {
			  if (output_array[i*COLS+j] != 0) {
				  num_failures++;
				  // if (output_array[i*COLS+j] != TEST_RX_INVALID_32)
				//	  xil_printf("At (%d, %d), expected 0x%x,\t Rx: 0x%x\r\n", i, j, 0, output_array[i*COLS+j]);
			  }
		  }

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
  xil_printf("-TX preparing %d BDs: %d\r\n", packet.NumTxBds, tx_prepare);
  xil_printf("Prepare data: %d\r\n", prepare_data);
  xil_printf("Stencil init: %d\r\n", accel_init);
  xil_printf("Stencil runtime: %d, spun %d times\r\n", accel_runtime, counter);
  xil_printf("Output check: %d\r\n", check_time);
  xil_printf("Total runtime: %d\r\n", total_runtime);


  if (num_failures > 0)
	  xil_printf("Failed %d times!\r\n", num_failures);
  else
	  xil_printf("Test passed!\r\n");

  cleanup_platform();
  return 0;
}
