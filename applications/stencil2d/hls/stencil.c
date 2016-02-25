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
#include "stencil.h"

void stencil(TYPE in_stream[row_size * col_size + f_size],
             TYPE out_stream[row_size * col_size]) {
#pragma HLS INTERFACE s_axilite bundle=BUS_A port=return
#pragma HLS INTERFACE axis port=in_stream bundle=INPUT_STREAM
#pragma HLS INTERFACE axis port=out_stream bundle=OUTPUT_STREAM
  int i, j, k1, k2, row;
  int elem;
  TYPE orig[row_size * col_size];
  TYPE sol[row_size * col_size];
  TYPE filter[f_size];
  for (i = 0; i < row_size; i++) {
    row = i * col_size;
    for (j = 0; j < col_size; j++) {
#pragma HLS PIPELINE II=1
      elem = row + j;
      orig[elem] = in_stream[elem];
      sol[elem] = 0;
    }
  }
  for (i = 0; i < f_size; i++) {
#pragma HLS PIPELINE II=1
    filter[i] = in_stream[row_size * col_size + i];
  }

  TYPE temp, mul;
  temp = 0;

stencil_label1:
  for (i = 0; i < row_size - 2; i++) {
    row = i * col_size;
  stencil_label2:
    for (j = 0; j < col_size - 2; j++) {
      temp = 0;
    stencil_label3:
      for (k1 = 0; k1 < 3; k1++) {
      stencil_label4:
        for (k2 = 0; k2 < 3; k2++) {
          mul = filter[k1 * 3 + k2] *
                orig[(i * col_size) + j + k1 * col_size + k2];
          temp += mul;
        }
      }
      sol[row + j] = temp;
    }
  }

  for (i = 0; i < row_size; i++) {
    row = i * col_size;
    for (j = 0; j < col_size; j++) {
#pragma HLS PIPELINE II=1
      elem = row + j;
      out_stream[elem] = sol[elem];
    }
  }
}
