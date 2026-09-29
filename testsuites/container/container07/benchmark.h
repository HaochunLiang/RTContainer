/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef CONTAINER07_BENCHMARK_H
#define CONTAINER07_BENCHMARK_H

/* Override with TEST_CONTAINER07_CPPFLAGS in the BSP configuration file. */
#ifndef C07_ROUNDS
#define C07_ROUNDS 3u
#endif
#ifndef C07_STARTUP_SAMPLES
#define C07_STARTUP_SAMPLES 30u
#endif
#ifndef C07_STARTUP_WARMUP
#define C07_STARTUP_WARMUP 3u
#endif
#ifndef C07_TIMER_SAMPLES
#define C07_TIMER_SAMPLES 30000u
#endif
#ifndef C07_TIMER_WARMUP
#define C07_TIMER_WARMUP 1000u
#endif
#ifndef C07_SWITCH_SAMPLES
#define C07_SWITCH_SAMPLES 20000u
#endif
#ifndef C07_SWITCH_WARMUP
#define C07_SWITCH_WARMUP 1000u
#endif
#ifndef C07_PERIOD_US
#define C07_PERIOD_US 1000u
#endif
#ifndef C07_TICK_US
#define C07_TICK_US 1000u
#endif
#ifndef C07_PRINT_RAW
#define C07_PRINT_RAW 1
#endif

#define C07_WORKER_PRIORITY 20u
#define C07_WAIT_SECONDS 30u
/* The implementation inserts these values into the TICKS watchdog queue. */
#define C07_CPU_BUDGET_TICKS 1000000u
#define C07_NS_PER_SECOND UINT64_C(1000000000)
#define C07_PERIOD_NS ((uint64_t) C07_PERIOD_US * 1000u)
#define C07_MAX(a, b) ((a) > (b) ? (a) : (b))
#define C07_SORT_CAPACITY \
  (C07_ROUNDS * C07_MAX(C07_TIMER_SAMPLES, \
    C07_MAX(C07_SWITCH_SAMPLES, C07_STARTUP_SAMPLES)))

#if C07_ROUNDS < 1 || C07_STARTUP_SAMPLES < 1 || \
    C07_TIMER_SAMPLES < 1 || C07_SWITCH_SAMPLES < 1
#error "container07 requires positive round and sample counts"
#endif
#if C07_STARTUP_WARMUP < 0 || C07_TIMER_WARMUP < 0 || C07_SWITCH_WARMUP < 0
#error "container07 warmup counts must be nonnegative"
#endif
#if C07_TICK_US < 1 || C07_PERIOD_US < C07_TICK_US || \
    C07_PERIOD_US % C07_TICK_US != 0
#error "container07 timer period must be an integer multiple of the clock tick"
#endif
#if C07_PRINT_RAW != 0 && C07_PRINT_RAW != 1
#error "C07_PRINT_RAW must be 0 or 1"
#endif

#endif
