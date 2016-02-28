#include "md.h"
#include <stdio.h>
#include <string.h>

int INPUT_SIZE = sizeof(struct bench_args_t);

#define EPSILON ((TYPE)1.0e-6)

union fptoi {
  float fp;
  int bits;
};

int convert(float fp) {
  union fptoi conv;
  conv.fp = fp;
  return conv.bits;
}

void run_benchmark_axis(void *vargs) {
  int i;
  TYPE hash;
  TYPE* in_stream, *out_stream;
  struct bench_args_float_t new_args;
  struct bench_args_t *args = (struct bench_args_t *)vargs;
  hash = 0;
  for (i = 0; i < nAtoms; i++) {
    new_args.position_x[i] = args->position_x[i];
    new_args.position_y[i] = args->position_y[i];
    new_args.position_z[i] = args->position_z[i];
  }
  for (i = 0; i < nAtoms*maxNeighbors; i++) {
    new_args.NL[i] = (TYPE)args->NL[i];
  }
#ifdef DEBUG
  for (i = 0; i < nAtoms; i++)
    printf("position_x[%d] = 0x%x\n", i, convert(new_args.position_x[i]));
  for (i = 0; i < nAtoms; i++)
    printf("position_y[%d] = 0x%0x\n", i, convert(new_args.position_y[i]));
  for (i = 0; i < nAtoms; i++)
    printf("position_z[%d] = 0x%0x\n", i, convert(new_args.position_z[i]));
  for (i = 0; i < nAtoms*maxNeighbors; i++)
    printf("NL[%d] = 0x%0x\n", i, convert(new_args.NL[i]));
#endif
  // TODO: Input stream starts from the positions, while outputs can go
  // straight to the force arrays. Obviously, not making separate storage for
  // input and output is dangerous and bad and evil...fix later.
  in_stream = (TYPE*) &new_args.position_x[0];
  out_stream = (TYPE*) &new_args.force_x[0];
  mdknn(in_stream, out_stream);

  for (i = 0; i < 3*nAtoms; i++) {
    hash += out_stream[i];
  }
  printf("Hash of output is %2.8f\n", hash);
}

void run_benchmark( void *vargs ) {
  run_benchmark_axis(vargs);
  /*
  struct bench_args_t *args = (struct bench_args_t *)vargs;
  md_kernel( args->force_x, args->force_y, args->force_z,
             args->position_x, args->position_y, args->position_z,
             args->NL );
   */
}

/* Input format:
%% Section 1
TYPE[nAtoms]: x positions
%% Section 2
TYPE[nAtoms]: y positions
%% Section 3
TYPE[nAtoms]: z positions
%% Section 4
int32_t[nAtoms*maxNeighbors]: neighbor list
*/

void input_to_data(int fd, void *vdata) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;
  char *p, *s;
  // Zero-out everything.
  memset(vdata,0,sizeof(struct bench_args_t));
  // Load input string
  p = readfile(fd);

  s = find_section_start(p,1);
  STAC(parse_,TYPE,_array)(s, data->position_x, nAtoms);

  s = find_section_start(p,2);
  STAC(parse_,TYPE,_array)(s, data->position_y, nAtoms);

  s = find_section_start(p,3);
  STAC(parse_,TYPE,_array)(s, data->position_z, nAtoms);

  s = find_section_start(p,4);
  parse_int32_t_array(s, data->NL, nAtoms*maxNeighbors);

}

void data_to_input(int fd, void *vdata) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;

  write_section_header(fd);
  STAC(write_,TYPE,_array)(fd, data->position_x, nAtoms);

  write_section_header(fd);
  STAC(write_,TYPE,_array)(fd, data->position_y, nAtoms);

  write_section_header(fd);
  STAC(write_,TYPE,_array)(fd, data->position_z, nAtoms);

  write_section_header(fd);
  write_int32_t_array(fd, data->NL, nAtoms*maxNeighbors);

}

/* Output format:
%% Section 1
TYPE[nAtoms]: new x force
%% Section 2
TYPE[nAtoms]: new y force
%% Section 3
TYPE[nAtoms]: new z force
*/

void output_to_data(int fd, void *vdata) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;
  char *p, *s;
  // Zero-out everything.
  memset(vdata,0,sizeof(struct bench_args_t));
  // Load input string
  p = readfile(fd);

  s = find_section_start(p,1);
  STAC(parse_,TYPE,_array)(s, data->force_x, nAtoms);

  s = find_section_start(p,2);
  STAC(parse_,TYPE,_array)(s, data->force_y, nAtoms);

  s = find_section_start(p,3);
  STAC(parse_,TYPE,_array)(s, data->force_z, nAtoms);

}

void data_to_output(int fd, void *vdata) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;

  write_section_header(fd);
  STAC(write_,TYPE,_array)(fd, data->force_x, nAtoms);

  write_section_header(fd);
  STAC(write_,TYPE,_array)(fd, data->force_y, nAtoms);

  write_section_header(fd);
  STAC(write_,TYPE,_array)(fd, data->force_z, nAtoms);
}

int check_data( void *vdata, void *vref ) {
  struct bench_args_t *data = (struct bench_args_t *)vdata;
  struct bench_args_t *ref = (struct bench_args_t *)vref;
  int has_errors = 0;
  int i;
  TYPE diff_x, diff_y, diff_z;

  for( i=0; i<nAtoms; i++ ) {
    diff_x = data->force_x[i] - ref->force_x[i];
    diff_y = data->force_y[i] - ref->force_y[i];
    diff_z = data->force_z[i] - ref->force_z[i];
    has_errors |= (diff_x<-EPSILON) || (EPSILON<diff_x);
    has_errors |= (diff_y<-EPSILON) || (EPSILON<diff_y);
    has_errors |= (diff_z<-EPSILON) || (EPSILON<diff_z);
  }

  // Return true if it's correct.
  return !has_errors;
}
