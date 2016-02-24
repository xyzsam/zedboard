#ifndef _ARM_TIMING_UTILS_H_
#define _ARM_TIMING_UTILS_H_

#ifndef __linux__
#define int32_t int
#endif

//void gettimeofday_us(int32_t* cycleLo, int32_t* cycleHi);
#ifdef __linux__
void gettimeofday_us(int32_t* cycleLo);
#endif
void pmu_config(int counter, int event);
unsigned int read_pmu(int counter);
unsigned int get_cyclecount();
void clear_perfcounters(int32_t reset, int32_t enable_divider);
void init_perfcounters(int32_t do_reset, int32_t enable_divider);
#endif
