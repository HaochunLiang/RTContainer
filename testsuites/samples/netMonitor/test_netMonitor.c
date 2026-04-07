/*
 * 网络监控基本测试
 *
 * 测试网络监控功能的基本功能
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

rtems_task Init(rtems_task_argument argument)
{
    printf("\n=== Network Monitor Basic Test ===\n");

#ifdef RTEMSCFG_MONITOR_NET
    printf("Network monitoring is enabled\n");
    
    /* 初始化监控系统 */
    rtems_monitor_initialize();
    printf("Monitor system initialized\n");
    
    /* 采样网络数据 */
    rtems_monitor_sample();
    printf("Network data sampled\n");
    
    /* 获取网络统计快照 */
    RtemsMonitor snapshot;
    rtems_monitor_get_snapshot(&snapshot);
    
    printf("Network Statistics:\n");
    printf("  RX Bytes: %llu\n", (unsigned long long)snapshot.net.rx_bytes);
    printf("  TX Bytes: %llu\n", (unsigned long long)snapshot.net.tx_bytes);
    printf("  RX Packets: %llu\n", (unsigned long long)snapshot.net.rx_packets);
    printf("  TX Packets: %llu\n", (unsigned long long)snapshot.net.tx_packets);
    printf("  RX Errors: %llu\n", (unsigned long long)snapshot.net.rx_errors);
    printf("  TX Errors: %llu\n", (unsigned long long)snapshot.net.tx_errors);
    
    /* 打印简单的一行输出 */
    rtems_monitor_print_line();
    
    printf("\nNetwork monitor test completed successfully!\n");
#else
    printf("Network monitoring is not enabled\n");
#endif

    printf("\n*** END OF NETWORK MONITOR TEST ***\n");
    exit(0);
}
