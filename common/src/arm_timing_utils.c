/*
 * Timing functions on ARMv7.
 *
 * The perf counters are 32-bit and must be enabled by a kernal module.
 */

#include <stdlib.h>
#include <sys/time.h>
#include "arm_timing_utils.h"

#ifdef __linux__
void gettimeofday_us(int32_t* cycleLo) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int32_t usecs = tv.tv_sec * 1e6 + tv.tv_usec;
  *cycleLo = usecs;
}
#endif


unsigned int get_cyclecount()
{
    unsigned int value;

    /* Read CCNT Register */
    asm volatile ("mrc p15, 0, %0, c9, c13, 0\t\n": "=r"(value));

    return value;
}

void pmu_config(int counter, int event) {
	// Leave only the last 5 bits.
	counter &= 0x1F;
	// Write PMSELR register (select one of the available counters).
	asm volatile ("mcr p15, 0, %0, c9, c12, 5\t\n" : "=r"(counter));
	// Synchronize context.
	asm volatile ("isb");
	// Program the event code.
	asm volatile ("mcr p15, 0, %0, c9, c13, 1\t\n": "=r"(event));
}

unsigned int read_pmu(int counter) {
	int value;
	// Leave only the last 5 bits.
	counter &= 0x1F;
	// Write PMSELR register (select one of the available counters).
	asm volatile ("mcr p15, 0, %0, c9, c12, 5\t\n" : "=r"(counter));
	// Synchronize context.
	asm volatile ("isb");
	// Read the current register.
	asm volatile ("mrc p15, 0, %0, c9, c13, 2\t\n": "=r"(value));
	return value;
}

void clear_perfcounters(int32_t do_reset, int32_t enable_divider) {
    /* In general enable all counters (including cycle counter) */
    int32_t value = 1;

    /* Peform reset */
    if (do_reset) {
        value |= 2; /* reset all counters to zero */
        value |= 4; /* reset cycle counter to zero */
    }

    if (enable_divider)
        value |= 8; /* enable "by 64" divider for CCNT */

    value |= 16;

    /* Program the performance-counter control-register */
    asm volatile ("mcr p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));
}

void init_perfcounters(int32_t do_reset, int32_t enable_divider)
{
    clear_perfcounters(do_reset, enable_divider);

    /* Enable all counters */
    asm volatile ("mcr p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000003f));

    /* Clear overflows */
    asm volatile ("mcr p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000003f));
}
