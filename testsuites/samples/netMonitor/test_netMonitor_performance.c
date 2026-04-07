/*
 * 网络监控性能测试
 *
 * 测试网络监控功能的性能开销
 */

#include <rtems.h>
#include <rtems/score/monitor.h>
#include <stdio.h>
#include <stdlib.h>

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_MAXIMUM_TASKS 2
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

rtems_task Init(rtems_task_argument argument);

static void measure_network_monitor_performance(void)
{
    printf("\n=== Network Monitor Performance Measurement ===\n");

#ifdef RTEMSCFG_MONITOR_NET
    /* 测量初始化性能 */
    uint32_t start_time = rtems_clock_get_ticks_since_boot();
    rtems_monitor_initialize();
    uint32_t end_time = rtems_clock_get_ticks_since_boot();
    printf("Monitor initialization took %llu ticks\n",
           (unsigned long long)(end_time - start_time));
    
    /* 测量采样性能 */
    const int sample_count = 1000;
    start_time = rtems_clock_get_ticks_since_boot();
    
    for (int i = 0; i < sample_count; i++) {
        rtems_monitor_sample();
    }
    
    end_time = rtems_clock_get_ticks_since_boot();
    uint32_t total_ticks = end_time - start_time;
    printf("Sampling %d times took %llu ticks\n",
           sample_count, (unsigned long long)total_ticks);
    printf("Average time per sample: %llu ticks\n",
           (unsigned long long)(total_ticks / sample_count));
    
    /* 测量快照获取性能 */
    RtemsMonitor snapshot;
    start_time = rtems_clock_get_ticks_since_boot();
    
    for (int i = 0; i < sample_count; i++) {
        rtems_monitor_get_snapshot(&snapshot);
    }
    
    end_time = rtems_clock_get_ticks_since_boot();
    total_ticks = end_time - start_time;
    printf("Getting %d snapshots took %llu ticks\n",
           sample_count, (unsigned long long)total_ticks);
    printf("Average time per snapshot: %llu ticks\n",
           (unsigned long long)(total_ticks / sample_count));
    
    /* 测量打印性能 */
    start_time = rtems_clock_get_ticks_since_boot();
    
    for (int i = 0; i < 100; i++) {
        rtems_monitor_print_line();
    }
    
    end_time = rtems_clock_get_ticks_since_boot();
    total_ticks = end_time - start_time;
    printf("Printing 100 lines took %llu ticks\n",
           (unsigned long long)total_ticks);
    printf("Average time per print: %llu ticks\n",
           (unsigned long long)(total_ticks / 100));
    
    printf("\nPerformance measurement completed!\n");
#else
    printf("Network monitoring is not enabled\n");
#endif
}

rtems_task Init(rtems_task_argument argument)
{
    printf("\n=== Network Monitor Performance Test ===\n");

#ifdef RTEMSCFG_MONITOR_NET
    printf("Network monitoring is enabled\n");
    
    /* 执行性能测试 */
    measure_network_monitor_performance();
    
    printf("\nNetwork monitor performance test completed successfully!\n");
#else
    printf("Network monitoring is not enabled\n");
#endif

    printf("\n*** END OF NETWORK MONITOR PERFORMANCE TEST ***\n");
    exit(0);
}
