/*
 * 网络监控详细测试
 *
 * 测试网络监控功能在不同网络活动下的表现
 */

#include <rtems.h>
#include <rtems/score/monitor.h>
#include <stdio.h>
#include <stdlib.h>

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_MAXIMUM_TASKS 3
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

rtems_task Init(rtems_task_argument argument);
rtems_task NetworkTask(rtems_task_argument argument);

static void print_network_snapshot(const char *phase)
{
    RtemsMonitor snapshot;

    /* 先采样数据，然后获取快照 */
    rtems_monitor_sample();
    rtems_monitor_get_snapshot(&snapshot);

    printf("\n=== Network Snapshot: %s ===\n", phase);

#ifdef RTEMSCFG_MONITOR_NET
    printf("Network Statistics:\n");
    printf("  RX Bytes: %llu\n", (unsigned long long)snapshot.net.rx_bytes);
    printf("  TX Bytes: %llu\n", (unsigned long long)snapshot.net.tx_bytes);
    printf("  RX Packets: %llu\n", (unsigned long long)snapshot.net.rx_packets);
    printf("  TX Packets: %llu\n", (unsigned long long)snapshot.net.tx_packets);
    printf("  RX Errors: %llu\n", (unsigned long long)snapshot.net.rx_errors);
    printf("  TX Errors: %llu\n", (unsigned long long)snapshot.net.tx_errors);
#endif
}

rtems_task NetworkTask(rtems_task_argument argument)
{
    printf("Network task started\n");
    
    /* 模拟一些网络活动 */
    for (int i = 0; i < 5; i++) {
        printf("Network activity cycle %d\n", i + 1);
        
        /* 等待一段时间 */
        rtems_task_wake_after(rtems_clock_get_ticks_per_second());
        
        /* 打印网络快照 */
        char phase[32];
        snprintf(phase, sizeof(phase), "Cycle %d", i + 1);
        print_network_snapshot(phase);
    }
    
    printf("Network task completed\n");
    rtems_task_delete(RTEMS_SELF);
}

rtems_task Init(rtems_task_argument argument)
{
    rtems_status_code status;
    rtems_id task_id;

    printf("\n=== Network Monitor Detailed Test ===\n");

#ifdef RTEMSCFG_MONITOR_NET
    printf("Network monitoring is enabled\n");
    
    /* 初始化监控系统 */
    rtems_monitor_initialize();
    printf("Monitor system initialized\n");
    
    /* 初始网络快照 */
    print_network_snapshot("Initial");
    
    /* 创建网络任务 */
    status = rtems_task_create(
        rtems_build_name('N', 'E', 'T', '1'),
        1,
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &task_id
    );
    
    if (status != RTEMS_SUCCESSFUL) {
        printf("Failed to create network task: %s\n", rtems_status_text(status));
        exit(1);
    }
    
    status = rtems_task_start(task_id, NetworkTask, 0);
    if (status != RTEMS_SUCCESSFUL) {
        printf("Failed to start network task: %s\n", rtems_status_text(status));
        exit(1);
    }
    
    /* 等待网络任务完成 */
    rtems_task_wake_after(rtems_clock_get_ticks_per_second() * 6);
    
    /* 最终网络快照 */
    print_network_snapshot("Final");
    
    printf("\nNetwork monitor detailed test completed successfully!\n");
#else
    printf("Network monitoring is not enabled\n");
#endif

    printf("\n*** END OF NETWORK MONITOR DETAILED TEST ***\n");
    exit(0);
}
