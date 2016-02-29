#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "nw.h"
#include "generate.h"

void generate_data(struct bench_args_t* data)
{
  // Must be exact length
  char* seqA = "tcgacgaaataggatgacagcacgttctcgtattagagggccgcggtacaaaccaaatgctgcggcgtacagggcacggggcgctgttcgggagatcgggggaatcgtggcgtgggtgattcgccggc";
  char* seqB = "ttcgagggcgcgtgtcgcggtccatcgacatgcccggtcggtgggacgtgggcgcctgatatagaggaatgcgattggaaggtcggacgggtcggcgagttgggcccggtgaatctgccatggtcgat";

  //printf("strlen(seqA) = %d, strlen(seqB) = %d\r\n", strlen(seqA), strlen(seqB));
  assert( ALEN==strlen(seqA) && "String initializers must be exact length");
  assert( BLEN==strlen(seqB) && "String initializers must be exact length");

  // Fill data structure
  memcpy(data->seqA, seqA, ALEN);
  memcpy(data->seqB, seqB, BLEN);
}
