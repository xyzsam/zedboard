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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define numStates 32
#define numObs 128

#ifdef GEM5_HARNESS
#include "gem5/aladdin_sys_connection.h"
#include "gem5/aladdin_sys_constants.h"
#endif

#ifdef DMA_MODE
#include "gem5/dma_interface.h"
#endif


// int viterbi(int Obs[numObs], float transMat[numStates*numObs], float obsLik[numStates*numObs], float v[numStates*numObs]);
int viterbi(float *in_stream);

////////////////////////////////////////////////////////////////////////////////
// Test harness interface code.

struct bench_args_t {
        int Obs[numObs];
        float transMat[numStates*numObs];
        float obsLik[numStates*numObs];
        float v[numStates*numObs];
};

struct bench_args_float_t {
        float Obs[numObs];
        float transMat[numStates*numObs];
        float obsLik[numStates*numObs];
        float v[numStates*numObs];
};

int INPUT_SIZE = sizeof(struct bench_args_t);

void run_benchmark( void *vargs ) {
  int i;
  struct bench_args_float_t *new_args = (struct bench_args_float_t*) vargs;
  int result;
  /*
  struct bench_args_t *args = (struct bench_args_t *)vargs;

  for (i = 0; i < numObs; i++)
    new_args.Obs[i] = (float)args->Obs[i];
  memcpy(&new_args.transMat[0], &args->transMat[0], numStates*numObs*sizeof(float));
  memcpy(&new_args.obsLik[0], &args->obsLik[0], numStates*numObs*sizeof(float));
  memcpy(&new_args.v[0], &args->v[0], numStates*numObs*sizeof(float));
  */

  float* in_stream = (float*)new_args;
  int final_state = viterbi(in_stream);
  printf("Final state: %d\n", final_state);
  // viterbi( args->Obs, args->transMat, args->obsLik, args->v);
}
////////////////////////////////////////////////////////////////////////////////
