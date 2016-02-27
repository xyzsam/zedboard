/*
 * profiling.h
 *
 *  Created on: Feb 23, 2016
 *      Author: Sam
 */

#ifndef PROFILING_H_
#define PROFILING_H_

extern unsigned dcache_flush;
extern unsigned dcache_invalidate;
extern unsigned dma_setup;
extern unsigned dma_invocation;
extern unsigned tx_prepare;
extern unsigned rx_prepare;
extern unsigned accel_init;
extern unsigned accel_runtime;
extern unsigned total_runtime;
extern unsigned prepare_data;
extern unsigned check_time;

#endif /* PROFILING_H_ */
