#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "lfsr.h"
#include "viterbi.h"
// Fake benchmark function to satisfy the extern
// int viterbi(int Obs[numObs], float transMat[numStates*numObs], float obsLik[numStates*numObs], float v[numStates*numObs]) { }

float RR(){
    float x = (float)((float)random()/(float)RAND_MAX);
    return x;
}

void generate_binary(struct bench_args_float_t *data)
{
  int i;
  // Fill data structure
  srandom(1);
  for(i=0; i<numObs; i++){
    data->Obs[i] = (float)(random()%numObs);
  }
  for(i=0; i<numObs*numStates; i++){
    data->transMat[i] = RR();
    data->obsLik[i] = RR();
    data->v[i] = 0;
  }
}
