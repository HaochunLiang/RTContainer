#include <stdio.h>
#include <rtems.h>
#include <rtems/score/monitor.h>
#include <rtems/cpuuse.h>
#include <tmacros.h>

const char rtems_test_name[] = "CPU MONITOR DETAILED TEST";

static rtems_task high_priority_task(rtems_task_argument arg)
{
    printf("High priority task %d started!\n", arg);
    
    // 高优先级任务做更多工作
    volatile int counter = 0;
    for (int i = 0; i < 2000000; ++i) {
        counter += i * i;
    }
    
    printf("High priority task %d completed!\n", arg);
    rtems_task_exit();
}

static rtems_task medium_priority_task(rtems_task_argument arg)
{
    printf("Medium priority task %d started!\n", arg);
    
    // 中等优先级任务做中等工作
    volatile int counter = 0;
    for (int i = 0; i < 1000000; ++i) {
        counter += i;
    }
    
    printf("Medium priority task %d completed!\n", arg);
    rtems_task_exit();
}

static rtems_task low_priority_task(rtems_task_argument arg)
{
    printf("Low priority task %d started!\n", arg);
    
    // 低优先级任务做少量工作
    volatile int counter = 0;
    for (int i = 0; i < 500000; ++i) {
        counter += 1;
    }
    
    printf("Low priority task %d completed!\n", arg);
    rtems_task_exit();
}

static void print_monitor_snapshot(const char *phase)
{
    RtemsMonitor snapshot;
    
    // 先采样数据，然后获取快照
    rtems_monitor_sample();
    rtems_monitor_get_snapshot(&snapshot);
    
    printf("\n=== Monitor Snapshot: %s ===\n", phase);
    
#ifdef RTEMSCFG_MONITOR_CPU
    printf("CPU Statistics:\n");
    printf("  Total runtime: %u.%06u seconds\n", 
           snapshot.cpu.total_seconds, snapshot.cpu.total_microseconds);
    printf("  Uptime since reset: %u.%06u seconds\n", 
           snapshot.cpu.uptime_seconds, snapshot.cpu.uptime_microseconds);
#endif
}

static rtems_task Init(rtems_task_argument ignored)
{
    rtems_id tid1, tid2, tid3, tid4, tid5;
    rtems_status_code sc;

    rtems_print_printer_fprintf_putc(&rtems_test_printer);
    TEST_BEGIN();

#ifdef RTEMSCFG_MONITOR_CPU
    printf("CPU Monitor Detailed Test started!\n");
    
    // 初始化监控系统
    rtems_monitor_initialize();
    printf("Monitor system initialized\n");
    
    // 重置 CPU 使用率统计
    rtems_cpu_usage_reset();
    printf("CPU usage statistics reset\n");
    
    // 初始快照
    rtems_monitor_sample();
    print_monitor_snapshot("Initial");
    
    // 创建高优先级任务
    sc = rtems_task_create(
        rtems_build_name('H','I','G','H'),
        1,  // 最高优先级
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid1
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid1, high_priority_task, 1);
        printf("High priority task created and started\n");
    }
    
    // 创建中等优先级任务
    sc = rtems_task_create(
        rtems_build_name('M','E','D','I'),
        2,  // 中等优先级
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid2
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid2, medium_priority_task, 1);
        printf("Medium priority task created and started\n");
    }
    
    // 创建低优先级任务
    sc = rtems_task_create(
        rtems_build_name('L','O','W','1'),
        3,  // 低优先级
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid3
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid3, low_priority_task, 1);
        printf("Low priority task 1 created and started\n");
    }
    
    // 等待一段时间让任务运行
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(200));
    
    // 采样并打印快照
    rtems_monitor_sample();
    print_monitor_snapshot("After First Batch");
    
    // 创建更多任务
    sc = rtems_task_create(
        rtems_build_name('H','I','G','2'),
        1,  // 最高优先级
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid4
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid4, high_priority_task, 2);
        printf("High priority task 2 created and started\n");
    }
    
    sc = rtems_task_create(
        rtems_build_name('L','O','W','2'),
        3,  // 低优先级
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid5
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid5, low_priority_task, 2);
        printf("Low priority task 2 created and started\n");
    }
    
    // 等待任务完成
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(500));
    
    // 最终采样
    print_monitor_snapshot("Final");
    
    // 打印简单监控信息
    printf("\n=== Simple Monitor Output ===\n");
    rtems_monitor_print_line();
    
    // 打印详细 CPU 使用率报告
    printf("\n=== Detailed CPU Usage Report ===\n");
    rtems_cpu_usage_report();
    
    // 打印详细监控报告
    printf("\n=== Detailed Monitor Report ===\n");
    rtems_monitor_print_detailed_report(&rtems_test_printer);
    
#else
    printf("RTEMSCFG_MONITOR_CPU not defined\n");
#endif

    TEST_END();
    rtems_test_exit(0);
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_MAXIMUM_TASKS            6
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
