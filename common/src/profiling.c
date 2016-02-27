/*
 * profiling.c
 *
 *  Created on: Feb 23, 2016
 *      Author: Sam
 */

#include "profiling.h"

unsigned dcache_flush;
unsigned dcache_invalidate;
unsigned dma_setup;
unsigned dma_invocation;
unsigned tx_prepare;
unsigned rx_prepare;
unsigned accel_runtime;
unsigned accel_init;
unsigned prepare_data;
unsigned check_time;
unsigned total_runtime;
