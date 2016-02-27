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

int viterbi(float *in_stream) {}

float RR(){
    float x = (float)((float)random_fake()/(float)RAND_MAX);
    return x;
}

union fptoi {
  float fp;
  int bits;
};

int convert(float fp) {
  union fptoi conv;
  conv.fp = fp;
  return conv.bits;
}


void generate_binary()
{
  struct bench_args_float_t data;
  char *ptr;
  int status, i, fd, written=0;

  // Fill data structure
  srandom_fake(1);
  for(i=0; i<numObs; i++){
    data.Obs[i] = (float)(random_fake()%numObs);
  }
  for(i=0; i<numObs*numStates; i++){
    data.transMat[i] = RR();
    data.obsLik[i] = RR();
    data.v[i] = 0;
  }
  union fptoi converter;
  for (i = 0; i < numObs; i++)
    printf("0x%x\n", convert(data.Obs[i]));
  for (i = 0; i < numObs * numStates; i++)
    printf("0x%x\n", convert(data.transMat[i]));
  for (i = 0; i < numObs * numStates; i++)
    printf("0x%x\n", convert(data.obsLik[i]));
  for (i = 0; i < numObs * numStates; i++)
    printf("0x%x\n", convert(data.v[i]));

  // Open and write
  fd = open("input.data", O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  assert( fd>0 && "Couldn't open input data file" );

  ptr = (char *) &data;
  while( written<sizeof(data) ) {
    status = write( fd, ptr, sizeof(data)-written );
    assert( status>=0 && "Couldn't write input data file" );
    written += status;
  }
}

int main(int argc, char **argv)
{
  generate_binary();
  return 0;
}
