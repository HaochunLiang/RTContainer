#include <stdio.h>
#include <rtems.h>
#include <rtems/score/monitor.h>
#include <rtems/cpuuse.h>
#include <tmacros.h>

const char rtems_test_name[] = "CPU MONITOR TEST";

static rtems_task cpu_intensive_task(rtems_task_argument arg)
{
    printf("CPU intensive task %d started!\n", arg);
    
    // 模拟 CPU 密集型工作
    volatile int counter = 0;
    for (int i = 0; i < 1000000; ++i) {
        counter += i;
    }
    
    printf("CPU intensive task %d completed!\n", arg);
    rtems_task_exit();
}

static rtems_task idle_task(rtems_task_argument arg)
{
    printf("Idle task %d started!\n", arg);
    
    // 模拟空闲工作
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));
    
    printf("Idle task %d completed!\n", arg);
    rtems_task_exit();
}

static rtems_task Init(rtems_task_argument ignored)
{
    rtems_id tid1, tid2, tid3;
    rtems_status_code sc;

    rtems_print_printer_fprintf_putc(&rtems_test_printer);
    TEST_BEGIN();

#ifdef RTEMSCFG_MONITOR_CPU
    printf("CPU Monitor test started!\n");
    
    // 初始化监控系统
    rtems_monitor_initialize();
    printf("Monitor system initialized\n");
    
    // 重置 CPU 使用率统计
    rtems_cpu_usage_reset();
    printf("CPU usage statistics reset\n");
    
    // 创建 CPU 密集型任务
    sc = rtems_task_create(
        rtems_build_name('C','P','U','1'),
        1,
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid1
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid1, cpu_intensive_task, 1);
        printf("CPU intensive task 1 created and started\n");
    } else {
        printf("Failed to create CPU intensive task 1: %s\n", rtems_status_text(sc));
    }
    
    // 创建空闲任务
    sc = rtems_task_create(
        rtems_build_name('I','D','L','1'),
        2,
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid2
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid2, idle_task, 1);
        printf("Idle task 1 created and started\n");
    } else {
        printf("Failed to create idle task 1: %s\n", rtems_status_text(sc));
    }
    
    // 再创建一个 CPU 密集型任务
    sc = rtems_task_create(
        rtems_build_name('C','P','U','2'),
        1,
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid3
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid3, cpu_intensive_task, 2);
        printf("CPU intensive task 2 created and started\n");
    } else {
        printf("Failed to create CPU intensive task 2: %s\n", rtems_status_text(sc));
    }
    
    // 等待任务完成
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(500));
    
    // 采样监控数据
    rtems_monitor_sample();
    printf("Monitor data sampled\n");
    
    // 打印简单监控信息
    printf("\n=== Simple Monitor Output ===\n");
    rtems_monitor_print_line();
    
    // 打印详细 CPU 使用率报告
    printf("\n=== Detailed CPU Usage Report ===\n");
    rtems_cpu_usage_report();
    
    // 获取 CPU 统计信息
    RtemsCpuStats cpu_stats;
    rtems_monitor_get_cpu(&cpu_stats);
    printf("\n=== CPU Statistics ===\n");
    printf("Total runtime: %u.%06u seconds\n", 
           cpu_stats.total_seconds, cpu_stats.total_microseconds);
    printf("Uptime since reset: %u.%06u seconds\n", 
           cpu_stats.uptime_seconds, cpu_stats.uptime_microseconds);
    
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
#define CONFIGURE_MAXIMUM_TASKS            4
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
