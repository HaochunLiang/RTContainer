#ifndef _RTEMS_SCORE_MONITOR_H
#define _RTEMS_SCORE_MONITOR_H

#include <rtems/printer.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RTEMSCFG_MONITOR_CPU
typedef struct {
  uint32_t total_seconds;
  uint32_t total_microseconds;
  uint32_t uptime_seconds;
  uint32_t uptime_microseconds;
} RtemsCpuStats;
#endif

#ifdef RTEMSCFG_MONITOR_MEM
typedef struct {
  uint64_t bytes_total;
  uint64_t bytes_used;
  uint64_t bytes_free;
  uint64_t bytes_high_water;
} RtemsMemStats;
#endif

#ifdef RTEMSCFG_MONITOR_NET
typedef struct {
  uint64_t rx_bytes;
  uint64_t tx_bytes;
  uint64_t rx_packets;
  uint64_t tx_packets;
  uint64_t rx_errors;
  uint64_t tx_errors;
} RtemsNetStats;
#endif

typedef struct {
#ifdef RTEMSCFG_MONITOR_CPU
  RtemsCpuStats cpu;
#endif
#ifdef RTEMSCFG_MONITOR_MEM
  RtemsMemStats mem;
#endif
#ifdef RTEMSCFG_MONITOR_NET
  RtemsNetStats net;
#endif
} RtemsMonitor;

void rtems_monitor_initialize(void);
void rtems_monitor_sample(void);
void rtems_monitor_get_snapshot(RtemsMonitor *out);

#ifdef RTEMSCFG_MONITOR_CPU
void rtems_monitor_get_cpu(RtemsCpuStats *out);
#endif

#ifdef RTEMSCFG_MONITOR_MEM
void rtems_monitor_get_mem(RtemsMemStats *out);
#endif

#ifdef RTEMSCFG_MONITOR_NET
void rtems_monitor_get_net(RtemsNetStats *out);
#endif

void rtems_monitor_print_line(void);
void rtems_monitor_print_detailed_report(const rtems_printer *printer);

#ifdef __cplusplus
}
#endif

#endif
