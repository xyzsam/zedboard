#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "kmp.h"
#include "TR.h"

void generate_data(struct bench_args_int32_t *data)
{
  memcpy(data->input, tr, STRING_SIZE);
  memcpy(data->pattern, "bull", PATTERN_SIZE);
}
