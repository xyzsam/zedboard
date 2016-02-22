/*
 * Measures the roundtrip time of an AXI4Lite transaction.
 * Author: Sam Xi
 */

#include <stdio.h>
#include "platform.h"
#include "xroundtrip.h"
#include "arm_timing_utils.h"

#define COUNTER_END 512

static XRoundtrip device;

int main() {
  int status;
  int counter, num_failures, num_it;
  int init_count_val = -1;
  int last_count_val = -1;
  int cycle_start, cycle_end, cycle_int;
  float total_cycles;
  init_platform();
  init_perfcounters(1, 0);

  status = XRoundtrip_Initialize(&device, XPAR_XROUNDTRIP_0_DEVICE_ID);
  if (status != XST_SUCCESS) {
    xil_printf("Could not initialize device with config.\r\n");
    return status;
  }

  // Get the initial counter value.
  XRoundtrip_Start(&device);
  init_count_val = XRoundtrip_Get_return(&device);
  last_count_val = init_count_val;
  num_failures = 0;
  num_it = 0;
  while (counter < init_count_val + COUNTER_END) {
    num_it++;
    XRoundtrip_Start(&device);
    cycle_start = get_cyclecount();
    counter = XRoundtrip_Get_return(&device);
    cycle_end = get_cyclecount();
    cycle_int = cycle_end - cycle_start;
    if (counter != last_count_val + 1)
      num_failures++;
    total_cycles += (cycle_int);
    last_count_val = counter;
  }
  printf("Number of iterations: %d\r\n", num_it);
  printf("Number of failures: %d\r\n", num_failures);
  printf("Average RTT: %2.4f\r\n", total_cycles / COUNTER_END);

  cleanup_platform();
  return 0;
}
