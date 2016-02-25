/*
 * For profiling cycle counts on regions of the benchmark.
 *
 * This organizes all the data so that we can store the cycle count now and
 * print it later.
 */

#ifndef PROFILING_H_
#define PROFILING_H_

extern unsigned dma_setup;
extern unsigned dma_invocation;
extern unsigned dcache_flush;
extern unsigned dcache_invalidate;
extern unsigned tx_prepare;
extern unsigned prepare_data;
extern unsigned check_time;
extern unsigned accel_runtime;
extern unsigned accel_init;
extern unsigned total_runtime;

#endif /* PROFILING_H_ */
