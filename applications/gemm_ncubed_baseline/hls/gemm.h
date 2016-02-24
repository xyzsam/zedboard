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

//Standard Libraries
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ncubed_args.h"

//Define the input range to operate over
#define MIN 2147483646
#define MAX -2147483646

//Set number of iterations to execute
#define MAX_ITERATION 1

#ifdef GEM5_HARNESS
#include "gem5/aladdin_sys_connection.h"
#include "gem5/aladdin_sys_constants.h"
#endif

#ifdef DMA_MODE
#include "gem5/dma_interface.h"
#endif

void gemm(TYPE m1[row_size * col_size * 2], TYPE prod[row_size * col_size]);

////////////////////////////////////////////////////////////////////////////////
// Test harness interface code.
int INPUT_SIZE = sizeof(struct bench_args_t);

void run_benchmark( void *vargs ) {
  struct bench_args_t *args = (struct bench_args_t *)vargs;
  TYPE mat[row_size * col_size * 2];
  bool tlast = false;
  int i, j;
  int status = 0;
  memset(&args->prod[0], 0, row_size*col_size*sizeof(int));

#ifdef GEM5_HARNESS
  mapArrayToAccelerator(
      MACHSUITE_GEMM_NCUBED, "m1", (void*)&args->m1, sizeof(args->m1));
  mapArrayToAccelerator(
      MACHSUITE_GEMM_NCUBED, "m2", (void*)&args->m2, sizeof(args->m2));
  mapArrayToAccelerator(
      MACHSUITE_GEMM_NCUBED, "prod", (void*)&args->prod, sizeof(args->prod));
  invokeAcceleratorAndBlock(MACHSUITE_GEMM_NCUBED);
#else
  // Add some fake data.
  for (i = 0; i < row_size; i++) {
    for (j = 0; j < row_size; j++) {
      args->m1[i*row_size + j] = 3;
      args->m2[i*row_size + j] = 0;
    }
  }
  for (i = 0; i < row_size; i++)
    args->m2[i*row_size + i] = 1;
  memcpy(&mat[0], &args->m1[0], row_size*col_size*sizeof(TYPE));
  memcpy(&mat[row_size*col_size], &args->m2[0], row_size*col_size*sizeof(TYPE));
  gemm(mat, args->prod);
#endif
}

////////////////////////////////////////////////////////////////////////////////
