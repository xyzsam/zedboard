#include "spmv.h"
#include <string.h>

int INPUT_SIZE = sizeof(struct bench_args_t);

#define EPSILON 1.0e-6

#ifdef ZYNQ
void run_benchmark_axi(void *vargs) {
  struct bench_args_t *args = (struct bench_args_t *)vargs;
  struct bench_args_float_t new_args;
  int i;
  TYPE hash;
  for (i = 0; i < NNZ; i++)
    new_args.val[i] = args->val[i];
  for (i = 0; i < NNZ; i++)
    new_args.cols[i] = (TYPE)args->cols[i];
  for (i = 0; i < N+1; i++)
    new_args.rowDelimiters[i] = (TYPE)args->rowDelimiters[i];
  for (i = 0; i < N; i++)
    new_args.vec[i] = args->vec[i];

  TYPE* in_stream = (TYPE*)&new_args;
  TYPE* out_stream = &(new_args.out[0]);

  spmv(in_stream, out_stream);
  memcpy(&args->out[0], &new_args.out[0], N*sizeof(TYPE));
  hash = 0;
  for (i = 0; i < N; i++)
    hash += new_args.out[i];
  printf("Hash: %2.8f\n", hash);
}
#endif

void run_benchmark( void *vargs ) {
#ifdef ZYNQ
  run_benchmark_axi(vargs);
#else
  struct bench_args_t *args = (struct bench_args_t *)vargs;
  spmv_kernel( args->val, args->cols, args->rowDelimiters, args->vec, args->out );
#endif
}

/* Input format:
%% Section 1
TYPE[NNZ]: the nonzeros of the matrix
%% Section 2
int32_t[NNZ]: the column index of the nonzeros
%% Section 3
int32_t[N+1]: the start of each row of nonzeros
%% Section 4
TYPE[N]: the dense vector
*/

void input_to_data(int fd, void *vdata) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;
  char *p, *s;
  // Zero-out everything.
  memset(vdata,0,sizeof(struct bench_args_t));
  // Load input string
  p = readfile(fd);

  s = find_section_start(p,1);
  STAC(parse_,TYPE,_array)(s, data->val, NNZ);

  s = find_section_start(p,2);
  parse_int32_t_array(s, data->cols, NNZ);

  s = find_section_start(p,3);
  parse_int32_t_array(s, data->rowDelimiters, N+1);

  s = find_section_start(p,4);
  STAC(parse_,TYPE,_array)(s, data->vec, N);
}

void data_to_input(int fd, void *vdata) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;

  write_section_header(fd);
  STAC(write_,TYPE,_array)(fd, data->val, NNZ);

  write_section_header(fd);
  write_int32_t_array(fd, data->cols, NNZ);

  write_section_header(fd);
  write_int32_t_array(fd, data->rowDelimiters, N+1);

  write_section_header(fd);
  STAC(write_,TYPE,_array)(fd, data->vec, N);
}

/* Output format:
%% Section 1
TYPE[N]: The output vector
*/

void output_to_data(int fd, void *vdata) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;
  char *p, *s;
  // Load input string
  p = readfile(fd);

  s = find_section_start(p,1);
  STAC(parse_,TYPE,_array)(s, data->out, N);
}

void data_to_output(int fd, void *vdata) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;

  write_section_header(fd);
  STAC(write_,TYPE,_array)(fd, data->out, N);
}

int check_data( void *vdata, void *vref ) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;
  struct bench_args_t *ref = (struct bench_args_t *)vref;
  int has_errors = 0;
  int i;
  TYPE diff;

  for(i=0; i<N; i++) {
    diff = data->out[i] - ref->out[i];
    has_errors |= (diff<-EPSILON) || (EPSILON<diff);
  }

  // Return true if it's correct.
  return !has_errors;
}
