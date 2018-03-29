/*
Implementation based on http://www-igm.univ-mlv.fr/~lecroq/string/node8.html
*/

#include "kmp.h"

void CPF(char pattern[PATTERN_SIZE], int32_t kmpNext[PATTERN_SIZE]) {
    int32_t k, q;
    k = 0;
    kmpNext[0] = 0;

    c1 : for(q = 1; q < PATTERN_SIZE; q++){
        c2 : while(k > 0 && pattern[k] != pattern[q]){
            k = kmpNext[q];
        }
        if(pattern[k] == pattern[q]){
            k++;
        }
        kmpNext[q] = k;
    }
}

int kmp_kernel(char pattern[PATTERN_SIZE], char input[STRING_SIZE],
               int32_t kmpNext[PATTERN_SIZE], int32_t n_matches[1]) {
    int32_t i, q;
    n_matches[0] = 0;

    CPF(pattern, kmpNext);

    q = 0;
    k1 : for(i = 0; i < STRING_SIZE; i++){
        k2 : while (q > 0 && pattern[q] != input[i]){
            q = kmpNext[q];
        }
        if (pattern[q] == input[i]){
            q++;
        }
        if (q >= PATTERN_SIZE){
            n_matches[0]++;
            q = kmpNext[q - 1];
        }
    }
    return 0;
}

int kmp(int32_t* in_stream) {
#pragma HLS INTERFACE s_axilite bundle=BUS_A port=return
#pragma HLS INTERFACE axis bundle=INPUT_STREAM depth=32415 port=in_stream
  char input[STRING_SIZE];
  char pattern[PATTERN_SIZE];
  int32_t kmpNext[PATTERN_SIZE];
  int32_t n_matches[1];
  int32_t temp;
  int i, j, q;

pattern_outer: for (i = 0; i < PATTERN_SIZE; i+=4) {
#pragma HLS PIPELINE II=1
    temp = in_stream[i/4];
pattern_inner:for (j = 0; j < 4; j++) {
#pragma HLS UNROLL
      pattern[i + j] = (temp >> (j*8)) & 0xff;
      kmpNext[j] = 0;
    }
  }

input_outer: for (i = 0; i < STRING_SIZE; i+=4) {
#pragma HLS PIPELINE II=1
    temp = in_stream[i/4+PATTERN_SIZE/sizeof(int32_t)];
input_inner: for (j = 0; j < 4; j++) {
#pragma HLS UNROLL
      input[i + j] = (temp >> (j*8)) & 0xff;
    }
  }

  // kmp_kernel(pattern, input, kmpNext, n_matches);
    n_matches[0] = 0;

    CPF(pattern, kmpNext);

    q = 0;
    k1 : for(i = 0; i < STRING_SIZE; i++){
        k2 : while (q > 0 && pattern[q] != input[i]){
            q = kmpNext[q];
        }
        if (pattern[q] == input[i]){
            q++;
        }
        if (q >= PATTERN_SIZE){
            n_matches[0]++;
            q = kmpNext[q - 1];
        }
    }

  return n_matches[0];
}
