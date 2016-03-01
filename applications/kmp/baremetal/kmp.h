#ifndef _KMP_H_
#define _KMP_H_

/*
Implementation based on http://www-igm.univ-mlv.fr/~lecroq/string/node8.html
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "support.h"

#define PATTERN_SIZE 4
#define STRING_SIZE (32411)


#ifdef ZYNQ
int kmp(int32_t * in_stream);
#endif

int kmp_kernel(char pattern[PATTERN_SIZE], char input[STRING_SIZE],
               int32_t kmpNext[PATTERN_SIZE], int32_t n_matches[1]);
////////////////////////////////////////////////////////////////////////////////
// Test harness interface code.

struct bench_args_t {
  char pattern[PATTERN_SIZE];
  char input[STRING_SIZE];
  int32_t kmpNext[PATTERN_SIZE];
  int32_t n_matches[1];
};

struct bench_args_int32_t {
  int32_t pattern[(PATTERN_SIZE)/sizeof(int32_t)];
  int32_t input[(STRING_SIZE+1)/sizeof(int32_t)];
  int32_t kmpNext[PATTERN_SIZE];
  int32_t n_matches[1];
};

#endif
