/*
Based on algorithm described here:
http://www.cs.berkeley.edu/~mhoemmen/matrix-seminar/slides/UCB_sparse_tutorial_1.pdf
*/

#include "spmv.h"

#ifdef ZYNQ
void spmv(TYPE* in_stream, TYPE* out_stream) {
#pragma HLS INTERFACE s_axilite bundle=BUS_A port=return
#pragma HLS INTERFACE axis bundle=INPUT_STREAM port=in_stream
#pragma HLS INTERFACE axis bundle=OUTPUT_STREAM port=out_stream
  TYPE val[NNZ];
  int32_t cols[NNZ];
  int32_t rowDelimiters[N + 1];
  TYPE vec[N];
  TYPE out[N];
  int i, j;
  TYPE sum, Si;

  for (i = 0; i < NNZ; i++) {
#pragma HLS PIPELINE II=1
    val[i] = in_stream[i];
  }
  for (i = 0; i < NNZ; i++) {
#pragma HLS PIPELINE II=1
    cols[i] = (int)in_stream[i+NNZ];
  }
  for (i = 0; i < N+1; i++) {
#pragma HLS PIPELINE II=1
    rowDelimiters[i] = (int)in_stream[i+2*NNZ];
  }
  for (i = 0; i < N; i++) {
#pragma HLS PIPELINE II=1
    vec[i] = in_stream[i+2*NNZ + N +1];
  }

  spmv_1 : for(i = 0; i < N; i++){
      sum = 0; Si = 0;
      int tmp_begin = rowDelimiters[i];
      int tmp_end = rowDelimiters[i+1];
      spmv_2 : for (j = tmp_begin; j < tmp_end; j++){
          Si = val[j] * vec[cols[j]];
          sum = sum + Si;
      }
      out[i] = sum;
  }

  for (i = 0; i < N; i++) {
#pragma HLS PIPELINE II=1
    out_stream[i] = out[i];
  }

}
#endif

void spmv_kernel(TYPE val[NNZ], int32_t cols[NNZ], int32_t rowDelimiters[N+1], TYPE vec[N], TYPE out[N]){
    int i, j;
    TYPE sum, Si;

    spmv_1 : for(i = 0; i < N; i++){
        sum = 0; Si = 0;
        int tmp_begin = rowDelimiters[i];
        int tmp_end = rowDelimiters[i+1];
        spmv_2 : for (j = tmp_begin; j < tmp_end; j++){
            Si = val[j] * vec[cols[j]];
            sum = sum + Si;
        }
        out[i] = sum;
    }
}


