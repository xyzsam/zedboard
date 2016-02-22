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
#include "xroundtrip.h"
#include "xparameters.h"
#include "arm_timing_utils.h"

#define PMU_COUNTER 1
#define COUNTER_END 512

static XRoundtrip device;

int main() {
  int status;
  int counter, i, num_failures, num_it;
  int init_count_val = -1;
  int last_count_val = -1;
  int cycle_start, cycle_end;
  volatile int inst_start, inst_end;
  float total_cycles;
  init_platform();
  pmu_config(PMU_COUNTER,
             0x68); // 0x08 = instructions architecturally executed.
  init_perfcounters(1, 0);

  // inst_start = read_pmu(PMU_COUNTER);
  cycle_start = get_cyclecount();
  status = XRoundtrip_Initialize(&device, XPAR_XROUNDTRIP_0_DEVICE_ID);
  if (status != XST_SUCCESS) {
    xil_printf("Could not initialize device with config.\r\n");
    return status;
  }
  cycle_end = get_cyclecount();
  // inst_end = read_pmu(PMU_COUNTER);

  printf("Instructions in initialization: %d, %d, took %d cycles\r\n", inst_end,
         inst_start, cycle_end - cycle_start);
  // Get the initial counter value.
  XRoundtrip_Start(&device);
  init_count_val = XRoundtrip_Get_return(&device);
  last_count_val = init_count_val;
  num_failures = 0;
  num_it = 0;
  while (counter < init_count_val + COUNTER_END) {
    num_it++;
    // inst_start = read_pmu(PMU_COUNTER);
    XRoundtrip_Start(&device);
    cycle_start = get_cyclecount();
    counter = XRoundtrip_Get_return(&device);
    cycle_end = get_cyclecount();
    // inst_end = read_pmu(PMU_COUNTER);
    if (counter != last_count_val + 1) {
      num_failures++;
    }
    total_cycles += (cycle_end - cycle_start);
    last_count_val = counter;
    // printf("Instructions in roundtrip: %d, %d\r\n", inst_end, inst_start);
  }
  printf("Number of iterations: %d\r\n", num_it);
  printf("Number of failures: %d\r\n", num_failures);
  printf("Average RTT: %2.4f\r\n", total_cycles / COUNTER_END);

  cleanup_platform();
  return 0;
}
