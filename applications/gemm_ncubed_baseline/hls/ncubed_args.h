#ifndef __AES_ARGS_H__
#define __AES_ARGS_H__

#include <stdint.h>

// Define compute data type
#define TYPE int

// Specify row/column sizes
#define row_size 64
#define col_size 64

struct bench_args_t {
  TYPE m1[row_size * col_size];
  TYPE m2[row_size * col_size];
  TYPE prod[row_size * col_size];
};

struct axistream_t {
  TYPE data;
  bool keep;
  bool last;
};

#endif
