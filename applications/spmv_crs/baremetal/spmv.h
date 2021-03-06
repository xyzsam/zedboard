/*
Based on algorithm described here:
http://www.cs.berkeley.edu/~mhoemmen/matrix-seminar/slides/UCB_sparse_tutorial_1.pdf
*/
#ifndef _SPMV_H_
#define _SPMV_H_

#include <stdlib.h>
#include <stdio.h>
#include "support.h"

// These constants valid for the IEEE 494 bus interconnect matrix
#define NNZ 1666
#define N 494

#define TYPE float

#ifdef ZYNQ
void spmv(TYPE* in_stream, TYPE* out_stream);
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
  TYPE val[NNZ];
  TYPE cols[NNZ];
  TYPE rowDelimiters[N+1];
  TYPE vec[N];
  TYPE out[N];
};

#endif
