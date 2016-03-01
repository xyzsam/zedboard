#include "kmp.h"
#include <string.h>

int INPUT_SIZE = sizeof(struct bench_args_t);

#ifdef ZYNQ
void run_benchmark_axi(void *vargs) {
  int32_t* in_stream;
  struct bench_args_int32_t new_args;
  struct bench_args_t *args = (struct bench_args_t *)vargs;
  int n_matches = 0;
  memcpy(&new_args.pattern[0], &args->pattern[0], PATTERN_SIZE);
  memcpy(&new_args.input[0], &args->input[0], STRING_SIZE);
  memset(&new_args.kmpNext[0], 0, PATTERN_SIZE);
  new_args.n_matches[0] = 0;
  in_stream = (int32_t*) &new_args;

  n_matches = kmp(in_stream);
  printf("Num matches: %d\n", n_matches);
  args->n_matches[0] = n_matches;
}
#endif

void run_benchmark( void *vargs ) {
#ifdef ZYNQ
  run_benchmark_axi(vargs);
#else
  struct bench_args_t *args = (struct bench_args_t *)vargs;
  kmp_kernel( args->pattern, args->input, args->kmpNext, args->n_matches );
#endif
}

/* Input format:
%% Section 1
char[PATTERN_SIZE]: pattern
%% Section 2
char[STRING_SIZE]: text
*/

void input_to_data(int fd, void *vdata) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;
  char *p, *s;
  // Zero-out everything.
  memset(vdata,0,sizeof(struct bench_args_t));
  // Load input string
  p = readfile(fd);

  s = find_section_start(p,1);
  parse_string(s, data->pattern, PATTERN_SIZE);

  s = find_section_start(p,2);
  parse_string(s, data->input, STRING_SIZE);
}

void data_to_input(int fd, void *vdata) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;

  write_section_header(fd);
  write_string(fd, data->pattern, PATTERN_SIZE);

  write_section_header(fd);
  write_string(fd, data->input, STRING_SIZE);
}

/* Output format:
%% Section 1
int[1]: number of matches
*/

void output_to_data(int fd, void *vdata) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;
  char *p, *s;
  // Zero-out everything.
  memset(vdata,0,sizeof(struct bench_args_t));
  // Load input string
  p = readfile(fd);

  s = find_section_start(p,1);
  parse_int32_t_array(s, data->n_matches, 1);
}

void data_to_output(int fd, void *vdata) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;

  write_section_header(fd); // No section header
  write_int32_t_array(fd, data->n_matches, 1);
}

int check_data( void *vdata, void *vref ) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;
  struct bench_args_t *ref = (struct bench_args_t *)vref;
  int has_errors = 0;

  has_errors |= (data->n_matches[0] != ref->n_matches[0]);

  // Return true if it's correct.
  return !has_errors;
}
