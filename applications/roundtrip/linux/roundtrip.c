#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>
#include <time.h>

#include "arm_timing_utils.h"
#include "xroundtrip.h"
static XRoundtrip instance;


void init_device() {
  int status = XRoundtrip_Initialize(&instance, "roundtrip_static_reg");
  if (status != XST_SUCCESS) {
    printf("Device initialization failed with error %d\n", status);
    return;
  }
  printf("Device initialized successfully!\n");
}

// Uses system wall clock time to measure the complete function.
float run_test_time(int num_tests) {
  uint32_t elapsed;
  float average;
  int i, val, ret, failures;
  elapsed = 0;
  average = 0;
  failures = 0;
  struct timeval start, end;

  gettimeofday(&start, NULL);
  for (i = 0; i < num_tests; i++) {
#ifdef ZYNQ
    XRoundtrip_Start_fast(&instance);
    while (!XRoundtrip_IsDone_fast(&instance)) ;
#endif
  }
  gettimeofday(&end, NULL);

  printf("Failures: %0.2f\n", ((float)failures)/num_tests);
  elapsed = (end.tv_sec * 1e6 + end.tv_usec) - (start.tv_sec * 1e6 - start.tv_usec);
  average = ((float) elapsed) / num_tests;
  return average;
}

// Directly reads the CCNT register for fine-grained profiling.
float run_test_ccnt(int num_tests) {
  uint32_t start_time, end_time, elapsed, total_elapsed;
  float average;
  int i, val, ret, failures;
  struct timespec sleep_time;
  failures = 0;
  elapsed = 0;
  total_elapsed = 0;
  average = 0;

  clear_perfcounters(1,0);
  for (i = 0; i < num_tests; i++) {
#ifdef ZYNQ
    start_time = get_cyclecount();
    XRoundtrip_Start_fast(&instance);
    while (!XRoundtrip_IsDone_fast(&instance));
    end_time = get_cyclecount();
    elapsed = (end_time - start_time);
    total_elapsed += elapsed;
#endif
  }

  printf("Failures: %0.2f\n", ((float)failures)/num_tests);
  average = ((float) (total_elapsed)) / num_tests;
  return average;
}

// Meant to be run under perf. It does not touch any of the performance
// counters.
float run_test_perf(int num_tests) {
  int i, val, ret, failures;
  failures = 0;

  for (i = 0; i < num_tests; i++) {
#ifdef ZYNQ
    XRoundtrip_Start_fast(&instance);
    while (!XRoundtrip_IsDone_fast(&instance));
#endif
  }

  return 0;
}

int main(int argc, char* argv[]) {
  float average_rtt;
  int num_tests;
  int mode = 0;

  if (argc != 3){
    printf("Usage: ./roundtrip [num_tests] [ccnt|time|perf]\n");
    return -1;
  }

  num_tests = atoi(argv[1]);
  average_rtt = 0;

  init_device();
  if (strncmp(argv[2], "ccnt", 4) == 0) {
    init_perfcounters(1,0);
    average_rtt = run_test_ccnt(num_tests);
    mode = 0;
  } else if (strncmp(argv[2], "time", 4) == 0) {
    average_rtt = run_test_time(num_tests);
    mode = 1;
  } else if (strncmp(argv[2], "perf", 4) == 0) {
    average_rtt = run_test_perf(num_tests);
    mode = 2;
  } else {
    return -1;
  }

  printf("Average round trip: %2.2f %s\n", average_rtt,
         mode == 0 ? "cycles" : mode == 1 ? "us" : mode == 2 ? "" : "NA");

  return 0;
}
