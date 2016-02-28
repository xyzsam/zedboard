#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "fft.h"

void generate_data(struct bench_args_t* data)
{
  int i;
  struct prng_rand_t state;

  // Fill data structure
  prng_srand(1,&state);
  for(i=0; i<512; i++){
    data->work_x[i] = ((TYPE)prng_rand(&state))/((TYPE)PRNG_RAND_MAX);
    data->work_y[i] = ((TYPE)prng_rand(&state))/((TYPE)PRNG_RAND_MAX);
  }
}
