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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Define input sizes
#define col_size 66
#define row_size 130
#define f_size 9

//Data Bounds
#define MIN 2147483646
#define MAX -2147483646
#define TYPE int

//Set number of iterations to execute
#define MAX_ITERATION 1

#ifdef GEM5_HARNESS
#include "gem5/aladdin_sys_connection.h"
#include "gem5/aladdin_sys_constants.h"
#endif

#ifdef DMA_MODE
#include "gem5/dma_interface.h"
#endif

#ifdef _SYNTHESIS_
void stencil (
    TYPE in_stream[row_size * col_size + f_size],
    TYPE out_stream[row_size * col_size]);
#else
void stencil( TYPE orig[row_size * col_size],
        TYPE sol[row_size * col_size],
        TYPE filter[f_size] );
#endif

////////////////////////////////////////////////////////////////////////////////
// Test harness interface code.

struct bench_args_t {
    TYPE orig[row_size*col_size];
    TYPE sol[row_size*col_size];
    TYPE filter[f_size];
};
int INPUT_SIZE = sizeof(struct bench_args_t);


void run_benchmark( void *vargs ) {
    int i, j;
    struct bench_args_t *args = (struct bench_args_t *)vargs;

#ifdef GEM5_HARNESS
  mapArrayToAccelerator(
      MACHSUITE_STENCIL_2D, "orig", (void*)&args->orig, sizeof(args->orig));
  mapArrayToAccelerator(
      MACHSUITE_STENCIL_2D, "sol", (void*)&args->sol, sizeof(args->sol));
  mapArrayToAccelerator(
      MACHSUITE_STENCIL_2D, "filter", (void*)&args->filter,
      sizeof(args->filter));
  invokeAcceleratorAndBlock(MACHSUITE_STENCIL_2D);
#endif
    for (i = 0; i < row_size; i++) {
      for (j = 0; j < col_size; j++) {
        args->orig[i*col_size+j] = 3;
      }
    }
    args->filter[0] = 0;
    args->filter[1] = 0;
    args->filter[2] = 0;
    args->filter[3] = 0;
    args->filter[4] = 1;
    args->filter[5] = 0;
    args->filter[6] = 0;
    args->filter[7] = 0;
    args->filter[8] = 0;
#ifdef _SYNTHESIS_
    TYPE in_stream[row_size*col_size + f_size];
    memcpy(&in_stream[0], &args->orig[0], row_size*col_size*sizeof(TYPE));
    memcpy(&in_stream[row_size*col_size], &args->filter[0], f_size*sizeof(TYPE));
    stencil(in_stream, args->sol);
#else
    stencil( args->orig, args->sol, args->filter );
#endif
}

////////////////////////////////////////////////////////////////////////////////
