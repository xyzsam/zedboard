/*
Copyright (c) 2014, the President and Fellows of Harvard College.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of Harvard University nor the names of its contributors may
  be used to endorse or promote products derived from this software without
  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gemm.h"

void gemm(TYPE m1[row_size * col_size * 2], TYPE prod[row_size * col_size]) {
#pragma HLS INTERFACE s_axilite bundle=BUS_A port=return
#pragma HLS INTERFACE axis port=m1 bundle=INPUT_STREAM
#pragma HLS INTERFACE axis port=prod bundle=OUTPUT_STREAM
  int i, j, k, i_row, elem;
  // Read in data sequentially (which is needed for stream IO).
  TYPE m1_inner[row_size * col_size];
  TYPE m2_inner[row_size * col_size];
  TYPE prod_inner[row_size * col_size];
  for (i = 0; i < row_size; i++) {
    i_row = i * col_size;
    for (j = 0; j < col_size; j++) {
#pragma HLS PIPELINE II=1
      m1_inner[i_row + j] = m1[i_row + j];
    }
  }
  for (i = row_size; i < 2 * row_size; i++) {
    i_row = i * col_size;
    for (j = 0; j < col_size; j++) {
#pragma HLS PIPELINE II=1
      m2_inner[i_row + j] = m1[i_row + j];
    }
  }
  TYPE mult, k_col;
  mult = 0;
  k_col = 0;
  i_row = 0;
outer:
  for (i = 0; i < row_size; i++) {
  middle:
    for (j = 0; j < col_size; j++) {
      i_row = i * col_size;
      elem = i_row + j;
      TYPE sum = 0; // prod[i_row + j];
    inner:
      for (k = 0; k < row_size; k++) {
#pragma HLS PIPELINE II=1
        k_col = k * col_size;
        mult = m1_inner[i_row + k] * m2_inner[k_col + j];
        sum += mult;
      }
      prod_inner[elem] = sum;
    }
  }
  for (i = 0; i < row_size; i++) {
    i_row = i * col_size;
    for (j = 0; j < col_size; j++) {
#pragma HLS PIPELINE II=1
      elem = i_row + j;
      prod[elem] = prod_inner[elem];
    }
  }
}
