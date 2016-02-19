/* Test a data copy fixed function block.
 *
 * This is an accelerator that simply copies data into an internal buffer and
 * then back out (presumably to a different location than the source). This is
 * in theory only slightly more complicated than a loopback, but it turns out
 * to exercise a lot more of the functionality of the DMA engine.
 *
 * Two implementations are given: with and without AXI4Stream sidechannel
 * signals. It can be shown that the DMA engine will only acknowledge
 * completion of a transaction if the TLAST and TKEEP signals are produced.
 */

#include <stdio.h>
#include "accel.h"


void init_arr(int in[ARR_SIZE], int out[ARR_SIZE]) {
  int i;
  for (i = 0; i < ARR_SIZE; i++) {
    in[i] = i;
    out[i] = -1;
  }
}

int check_arr(int in[ARR_SIZE], int out[ARR_SIZE]) {
  int i;
  int num_failures = 0;
  for (i = 0; i < ARR_SIZE; i++) {
    if (in[i] != out[i])
      num_failures++;
  }
  return num_failures;
}

void init_axi(struct axistream_t in[ARR_SIZE],
              struct axistream_t out[ARR_SIZE]) {
  int i;
  for (i = 0; i < ARR_SIZE; i++) {
    in[i].data = i;
    out[i].data = -1;
  }
}

int check_axi(struct axistream_t in[ARR_SIZE],
               struct axistream_t out[ARR_SIZE]) {
  int i;
  int num_failures = 0;
  for (i = 0; i < ARR_SIZE; i++) {
    if (in[i].data != out[i].data)
      num_failures++;
  }
  return num_failures;
}

int main() {
  int num_failures;
#ifdef ENA_SIDECHANNELS
  printf("AXI4Stream sidechannels enabled!\n");
  struct axistream_t array[ARR_SIZE];
  struct axistream_t outarray[ARR_SIZE+1];
  init_axi(array, outarray);
  copy_axi(array, outarray);
  num_failures = check_axi(array, outarray);
#else
  printf("AXI4Stream sidechannels not enabled!\n");
  int array[ARR_SIZE];
  int outarray[ARR_SIZE];
  init_arr(array, outarray);
  copy_arr(array, outarray);
  num_failures = check_arr(array, outarray);
#endif
  if (num_failures > 0) {
    printf("Num failures: %d\n", num_failures);
    return -1;
  } else {
    printf("Test succeeded.\n");
    return 0;
  }
}
