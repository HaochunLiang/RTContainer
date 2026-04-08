#include <stdio.h>
#include <rtems.h>
#include <rtems/score/monitor.h>
#include <rtems/libcsupport.h>
#include <tmacros.h>

const char rtems_test_name[] = "MEMORY MONITOR TEST";

static rtems_task memory_intensive_task(rtems_task_argument arg)
{
    printf("Memory intensive task %d started!\n", arg);
    
    // 分配一些内存
    void *ptr1 = malloc(1024);
    void *ptr2 = malloc(2048);
    void *ptr3 = malloc(4096);
    
    if (ptr1 && ptr2 && ptr3) {
        printf("Memory intensive task %d allocated memory successfully\n", arg);
        
        // 使用内存
        memset(ptr1, 0xAA, 1024);
        memset(ptr2, 0xBB, 2048);
        memset(ptr3, 0xCC, 4096);
        
        // 等待一段时间
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));
        
        // 释放部分内存
        free(ptr1);
        printf("Memory intensive task %d freed ptr1\n", arg);
        
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));
        
        // 释放剩余内存
        free(ptr2);
        free(ptr3);
        printf("Memory intensive task %d freed all memory\n", arg);
    } else {
        printf("Memory intensive task %d failed to allocate memory\n", arg);
    }
    
    printf("Memory intensive task %d completed!\n", arg);
    rtems_task_exit();
}

static rtems_task idle_task(rtems_task_argument arg)
{
    printf("Idle task %d started!\n", arg);
    
    // 模拟空闲工作
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(50));
    
    printf("Idle task %d completed!\n", arg);
    rtems_task_exit();
}

static rtems_task Init(rtems_task_argument ignored)
{
    rtems_id tid1, tid2, tid3;
    rtems_status_code sc;

    rtems_print_printer_fprintf_putc(&rtems_test_printer);
    TEST_BEGIN();

#ifdef RTEMSCFG_MONITOR_MEM
    printf("Memory Monitor test started!\n");
    
    // 初始化监控系统
    rtems_monitor_initialize();
    printf("Monitor system initialized\n");
    
    // 创建内存密集型任务
    sc = rtems_task_create(
        rtems_build_name('M','E','M','1'),
        1,
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid1
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid1, memory_intensive_task, 1);
        printf("Memory intensive task 1 created and started\n");
    } else {
        printf("Failed to create memory intensive task 1: %s\n", rtems_status_text(sc));
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
    
    // 再创建一个内存密集型任务
    sc = rtems_task_create(
        rtems_build_name('M','E','M','2'),
        1,
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid3
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid3, memory_intensive_task, 2);
        printf("Memory intensive task 2 created and started\n");
    } else {
        printf("Failed to create memory intensive task 2: %s\n", rtems_status_text(sc));
    }
    
    // 等待任务完成
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(500));
    
    // 采样监控数据
    rtems_monitor_sample();
    printf("Monitor data sampled\n");
    
    // 打印简单监控信息
    printf("\n=== Simple Monitor Output ===\n");
    rtems_monitor_print_line();
    
    // 获取内存统计信息
    RtemsMemStats mem_stats;
    rtems_monitor_get_mem(&mem_stats);
    printf("\n=== Memory Statistics ===\n");
    printf("Total memory: %llu bytes\n", (unsigned long long)mem_stats.bytes_total);
    printf("Used memory: %llu bytes\n", (unsigned long long)mem_stats.bytes_used);
    printf("Free memory: %llu bytes\n", (unsigned long long)mem_stats.bytes_free);
    printf("Largest free block: %llu bytes\n", (unsigned long long)mem_stats.bytes_high_water);
    
    // 打印详细监控报告
    printf("\n=== Detailed Monitor Report ===\n");
    rtems_monitor_print_detailed_report(&rtems_test_printer);
    
#else
    printf("RTEMSCFG_MONITOR_MEM not defined\n");
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
