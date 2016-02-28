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
#include "xcopy_axi.h"
#include "xscugic.h"

#include "dma_utils.h"

static XAxiDma dma_device;
static XCopy_axi copy_device;

int init_copy(XCopy_axi *copy_device, int copy_device_id) {
  int status = 0;
  XCopy_axi_Config *cfg;
  cfg = XCopy_axi_LookupConfig(copy_device_id);
  if (!cfg) {
    xil_printf("No hardware config found for AXI DMA device id %d\r\n",
               copy_device_id);
    return -1;
  }

  status = XCopy_axi_CfgInitialize(copy_device, cfg);
  if (status != XST_SUCCESS) {
    xil_printf("Failed to initialize device with error %d\r\n", status);
    return -1;
  }
  return 0;
}

void init_buffers(int *in, int *out, size_t num_elem) {
  int i;
  for (i = 0; i < num_elem; i++) {
    in[i] = i;
    out[i] = -1;
  }
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
  int status, i, num_failures;
  struct DmaChannel channel;
  struct DmaPacket packet;
  size_t num_bytes = 256 * sizeof(int);
  unsigned int *input_array;
  unsigned int *output_array;
  init_platform();
  InitDma(&dma_device, &channel, XPAR_AXIDMA_0_DEVICE_ID);
  init_copy(&copy_device, XPAR_COPY_AXI_0_DEVICE_ID);
  InitPacket(&packet, &channel);
  status = TxSetup(&dma_device, &channel);
  CHECK_STATUS_AND_QUIT(status, "tx setup failed!\r\n");
  status = RxSetup(&dma_device, &packet);
  CHECK_STATUS_AND_QUIT(status, "rx setup failed!\r\n");

  memset((void *)packet.TxBuf, TEST_TX_INVALID_VALUE,
         RX_BUFFER_BASE - TX_BUFFER_BASE);
  memset((void *)packet.RxBuf, TEST_RX_INVALID_VALUE,
         RX_BUFFER_HIGH - RX_BUFFER_BASE + 1);

  input_array = (unsigned int *)packet.TxBuf;
  output_array = (unsigned int *)packet.RxBuf;
  for (i = 0; i < num_bytes; i += 4) {
    packet.TxBuf[i] = i / 4;
  }
  packet.TxNumBytes = num_bytes;

  status = PreparePacket(&packet);
  CHECK_STATUS_AND_QUIT(status, "preparing packet failed!\r\n");

  XCopy_axi_Start(&copy_device);
  status = SendPacket(&packet);
  CHECK_STATUS_AND_QUIT(status, "sending packet failed!\r\n");
  while (!XCopy_axi_IsDone(&copy_device)) {}
  // status = WaitForCompletion(&packet);
  CHECK_STATUS_AND_QUIT(status, "waiting for completion failed failed!\r\n");

  num_failures = 0;

  for (i = 0; i < num_bytes; i++) {
    if (packet.TxBuf[i] != packet.RxBuf[i]) {
      num_failures++;
    } else {
      xil_printf("At i = %d, Tx: 0x%x\t, Rx: 0x%x\r\n", i, packet.TxBuf[i],
                 packet.RxBuf[i]);
    }
  }
  if (num_failures > 0)
    xil_printf("Failed %d times!\r\n", num_failures);
  else
    xil_printf("Test passed!\r\n");

  cleanup_platform();
  return 0;
}
