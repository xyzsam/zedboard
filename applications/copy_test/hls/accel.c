#include <stdbool.h>
#include "accel.h"

void copy_axi(struct axistream_t in_stream[ARR_SIZE],
              struct axistream_t out_stream[ARR_SIZE]) {
#ifdef _SYNTHESIS_
#pragma HLS INTERFACE s_axilite bundle=BUS_A port=return
#pragma HLS INTERFACE axis port=in_stream bundle=INPUT_STREAM
#pragma HLS INTERFACE axis port=out_stream bundle=OUTPUT_STREAM
#endif
  int i;
  int temp[ARR_SIZE];

  for (i = 0; i < ARR_SIZE; i++) {
#ifdef _SYNTHESIS_
#pragma HLS PIPELINE II=1
#endif
    temp[i] = in_stream[i].data;
  }

  for (i = 0; i < ARR_SIZE; i++) {
#ifdef _SYNTHESIS_
#pragma HLS PIPELINE II=1
#endif
    out_stream[i].data = temp[i];
    out_stream[i].keep = true;
    out_stream[i].last = (i == ARR_SIZE-1);
  }
}

void copy_arr(int in_stream[ARR_SIZE], int out_stream[ARR_SIZE]) {
#ifdef _SYNTHESIS_
#pragma HLS INTERFACE s_axilite bundle=BUS_A port=return
#pragma HLS INTERFACE axis port=in_stream bundle=INPUT_STREAM
#pragma HLS INTERFACE axis port=out_stream bundle=OUTPUT_STREAM
#endif
  int i;
  int temp[ARR_SIZE];

  for (i = 0; i < ARR_SIZE; i++) {
#ifdef _SYNTHESIS_
#pragma HLS PIPELINE II=1
#endif
    temp[i] = in_stream[i];
  }

  for (i = 0; i < ARR_SIZE; i++) {
#ifdef _SYNTHESIS_
#pragma HLS PIPELINE II=1
#endif
    out_stream[i] = temp[i];
  }
}
