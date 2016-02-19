#ifndef _ACCEL_H_
#define _ACCEL_H_

#include <stdbool.h>

#define ARR_SIZE 256

struct axistream_t {
  int data;
  bool keep;
  bool last;
};

void copy_axi(struct axistream_t in[ARR_SIZE],
              struct axistream_t out[ARR_SIZE]);
void copy_arr(int in[ARR_SIZE], int out[ARR_SIZE]);

#endif
