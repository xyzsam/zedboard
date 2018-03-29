/*
Based on algorithm described here:
http://www.cs.berkeley.edu/~mhoemmen/matrix-seminar/slides/UCB_sparse_tutorial_1.pdf
*/

#include "spmv.h"

#ifdef ZYNQ
void spmv(STREAM_TYPE* in_stream, STREAM_TYPE* out_stream) {
#pragma HLS INTERFACE s_axilite bundle=BUS_A port=return DEPTH=1024
#pragma HLS INTERFACE axis bundle=INPUT_STREAM port=in_stream DEPTH=1024
#pragma HLS INTERFACE axis bundle=OUTPUT_STREAM port=out_stream DEPTH=1024
  TYPE val[NNZ];
  int32_t cols[NNZ];
  int32_t rowDelimiters[N + 1];
  TYPE vec[N];
  TYPE out[N];
  int i, j;
  TYPE sum, Si;

  for (i = 0; i < NNZ; i++) {
#pragma HLS PIPELINE II=1
    STREAM_TYPE temp;
    temp = in_stream[i];
    val[i] = temp.fp;
    // printf("val[%d] = %x, %2.2f, %2.2f\n", i, temp.bits, temp.fp, val[i]);
    // val[i] = in_stream[i].fp;
  }
  for (i = 0; i < NNZ; i++) {
#pragma HLS PIPELINE II=1
    cols[i] = in_stream[i+NNZ].bits;
  }
  for (i = 0; i < N+1; i++) {
#pragma HLS PIPELINE II=1
    rowDelimiters[i] = in_stream[i+2*NNZ].bits;
  }
  for (i = 0; i < N; i++) {
#pragma HLS PIPELINE II=1
    STREAM_TYPE temp;
    temp = in_stream[i+2*NNZ + N + 1];
    vec[i] = temp.fp;
    // printf("vec[%d] = %x, %2.2f, %2.2f\n", i, temp.bits, temp.fp, vec[i]);
    // vec[i] = in_stream[i+2*NNZ + N +1].fp;
  }

  spmv_1 : for(i = 0; i < N; i++){
      sum = 0; Si = 0;
      int tmp_begin = rowDelimiters[i];
      int tmp_end = rowDelimiters[i+1];
      spmv_2 : for (j = tmp_begin; j < tmp_end; j++){
          Si = val[j] * vec[cols[j]];
          sum = sum + Si;
          // printf("sum = %2.2f\n", sum);
          // printf("val[%d] = %2.8f, cols[%d] = %d, vec[%d] = %2.8f\n", j, val[j], j, cols[j], cols[j], vec[cols[j]]);
      }
      out[i] = sum;
  }

  for (i = 0; i < N; i++) {
#pragma HLS PIPELINE II=1
    STREAM_TYPE temp;
    temp.fp = out[i];
    out_stream[i] = temp;
    // out_stream[i].fp = out[i];
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


