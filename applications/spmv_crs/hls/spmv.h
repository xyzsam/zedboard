/*
Based on algorithm described here:
http://www.cs.berkeley.edu/~mhoemmen/matrix-seminar/slides/UCB_sparse_tutorial_1.pdf
*/

#include <stdlib.h>
#include <stdio.h>
#include "support.h"

// These constants valid for the IEEE 494 bus interconnect matrix
#define NNZ 1666
#define N 494

union data_t {
  unsigned int bits;
  float fp;
};

#define TYPE float
#define STREAM_TYPE union data_t

#ifdef ZYNQ
void spmv(STREAM_TYPE* in_stream, STREAM_TYPE* out_stream);
#endif

void spmv_kernel(TYPE val[NNZ], int32_t cols[NNZ], int32_t rowDelimiters[N + 1],
          TYPE vec[N], TYPE out[N]);
////////////////////////////////////////////////////////////////////////////////
// Test harness interface code.

struct bench_args_t {
  TYPE val[NNZ];
  int32_t cols[NNZ];
  int32_t rowDelimiters[N+1];
  TYPE vec[N];
  TYPE out[N];
};

struct bench_args_float_t {
  STREAM_TYPE val[NNZ];
  STREAM_TYPE cols[NNZ];
  STREAM_TYPE rowDelimiters[N+1];
  STREAM_TYPE vec[N];
  STREAM_TYPE out[N];
};
