#ifndef __ZYNQ_H__
#define __ZYNQ_H__

#include "xgemm.h"
#include "ncubed_args.h"
#include "arm_timing_utils.h"

// Measure cycles if not building a release binary; otherwise, skip the
// measurement overhead.
#ifndef RELEASE
#define MEASURE_CYCLES(name, code) { \
  clear_perfcounters(1, 1); \
  unsigned name##_start = get_cyclecount(); \
  code; \
  unsigned name##_end = get_cyclecount(); \
  unsigned name##_elapsed = name##_end - name##_start; \
  printf(#name" elapsed time: %d\n", name##_elapsed); \
}
#else
#define MEASURE_CYCLES(name, code) { \
  code; \
}
#endif

static XGemm instance;

void initialize_pl() {
  int status = 0;
  status = XGemm_Initialize(&instance, "gemm_ncubed_baseline");
  if (status != XST_SUCCESS) {
    printf("Failed to initialize PL with status %d\n", status);
    return;
  }
  printf("Device was successfully initialized!\n");
  printf("Bus base address is: 0x%x\n", instance.Bus_a_BaseAddress);

  init_perfcounters(1, 1);
  printf("Performance counters enabled.\n");
}

void reset_pl() {
  size_t sz = row_size * col_size;
  int zerobuf[sz];
  memset(&zerobuf[0], 0, sz*sizeof(int));
  XGemm_Write_prod_Words(&instance, 0, &zerobuf[0], sz);
}

void copy_data_to_pl(struct bench_args_t* args) {
  // printf("Copying data to accelerator.\n");
  XGemm_Write_m1_Words(&instance, 0, &args->m1[0], row_size * col_size);
  XGemm_Write_m2_Words(&instance, 0, &args->m2[0], row_size * col_size);
}

void start_pl() {
  // printf("Starting accelerator.\n");
	XGemm_Start(&instance);
}

int wait_pl() {
  int i = 0;
  // printf("Waiting");
	while (!XGemm_IsDone(&instance)) {
    i++;
    /*
    if (i % 100 == 0) {
      // printf(".");
    }
    */
  }
  return i;
  // printf("Done! i = %d.\n", i);
}

void read_data_from_pl(struct bench_args_t* args) {
  // printf("Reading data from accelerator.\n");
  XGemm_Read_prod_Words(&instance, 0, &args->prod[0], row_size * col_size);
}

void release_pl() {
  XGemm_Release(&instance);
}

void memtest(int src[row_size * col_size], int buf[row_size * col_size]) {
  int r, c;
  for (r = 0; r < row_size; r++) {
    for (c = 0; c < col_size; c++) {
      buf[r * col_size + c] = src[r * col_size + c];
    }
  }
}

void run_on_zynq(struct bench_args_t* args) {
  int buf[row_size * col_size];
  MEASURE_CYCLES(run_1, memtest(args->m1, buf));
  MEASURE_CYCLES(run_2, memtest(args->m1, buf));
  MEASURE_CYCLES(init, initialize_pl());
  MEASURE_CYCLES(reset, reset_pl());
  MEASURE_CYCLES(copy, copy_data_to_pl(args));
  start_pl();
  MEASURE_CYCLES(run, wait_pl());
  MEASURE_CYCLES(read, read_data_from_pl(args));
  MEASURE_CYCLES(release, release_pl());
}

#endif
