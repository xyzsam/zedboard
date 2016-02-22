#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>
#include <time.h>

// The accelerator.
int32_t roundtrip() {
#pragma HLS INTERFACE s_axilite bundle=BUS_A port=return
  static int32_t in;
  in += 1;
  return in;
}

int main(int argc, char* argv[]) {
  int res;
  res = roundtrip();
  if (res == 1)
    return 0;
  return -1;
}
