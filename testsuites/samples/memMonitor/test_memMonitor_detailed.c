#include <stdio.h>
#include <rtems.h>
#include <rtems/score/monitor.h>
#include <rtems/libcsupport.h>
#include <tmacros.h>

const char rtems_test_name[] = "MEMORY MONITOR DETAILED TEST";

static rtems_task large_memory_task(rtems_task_argument arg)
{
    printf("Large memory task %d started!\n", arg);
    
    // 分配大量内存
    void *ptrs[10];
    size_t sizes[] = {1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288};
    
    for (int i = 0; i < 10; ++i) {
        ptrs[i] = malloc(sizes[i]);
        if (ptrs[i]) {
            printf("Large memory task %d allocated %zu bytes\n", arg, sizes[i]);
            memset(ptrs[i], 0xAA + i, sizes[i]);
        } else {
            printf("Large memory task %d failed to allocate %zu bytes\n", arg, sizes[i]);
        }
    }
    
    // 等待一段时间
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(200));
    
    // 释放一半内存
    for (int i = 0; i < 5; ++i) {
        if (ptrs[i]) {
            free(ptrs[i]);
            printf("Large memory task %d freed %zu bytes\n", arg, sizes[i]);
        }
    }
    
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(200));
    
    // 释放剩余内存
    for (int i = 5; i < 10; ++i) {
        if (ptrs[i]) {
            free(ptrs[i]);
            printf("Large memory task %d freed %zu bytes\n", arg, sizes[i]);
        }
    }
    
    printf("Large memory task %d completed!\n", arg);
    rtems_task_exit();
}

static rtems_task medium_memory_task(rtems_task_argument arg)
{
    printf("Medium memory task %d started!\n", arg);
    
    // 分配中等大小的内存
    void *ptr1 = malloc(8192);
    void *ptr2 = malloc(16384);
    void *ptr3 = malloc(32768);
    
    if (ptr1 && ptr2 && ptr3) {
        printf("Medium memory task %d allocated memory successfully\n", arg);
        
        // 使用内存
        memset(ptr1, 0xBB, 8192);
        memset(ptr2, 0xCC, 16384);
        memset(ptr3, 0xDD, 32768);
        
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(150));
        
        // 释放内存
        free(ptr1);
        free(ptr2);
        free(ptr3);
        printf("Medium memory task %d freed all memory\n", arg);
    } else {
        printf("Medium memory task %d failed to allocate memory\n", arg);
    }
    
    printf("Medium memory task %d completed!\n", arg);
    rtems_task_exit();
}

static rtems_task small_memory_task(rtems_task_argument arg)
{
    printf("Small memory task %d started!\n", arg);
    
    // 分配小内存
    void *ptr1 = malloc(256);
    void *ptr2 = malloc(512);
    void *ptr3 = malloc(1024);
    
    if (ptr1 && ptr2 && ptr3) {
        printf("Small memory task %d allocated memory successfully\n", arg);
        
        // 使用内存
        memset(ptr1, 0xEE, 256);
        memset(ptr2, 0xFF, 512);
        memset(ptr3, 0x11, 1024);
        
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));
        
        // 释放内存
        free(ptr1);
        free(ptr2);
        free(ptr3);
        printf("Small memory task %d freed all memory\n", arg);
    } else {
        printf("Small memory task %d failed to allocate memory\n", arg);
    }
    
    printf("Small memory task %d completed!\n", arg);
    rtems_task_exit();
}

static void print_monitor_snapshot(const char *phase)
{
    RtemsMonitor snapshot;
    
    // 先采样数据，然后获取快照
    rtems_monitor_sample();
    rtems_monitor_get_snapshot(&snapshot);
    
    printf("\n=== Monitor Snapshot: %s ===\n", phase);
    
#ifdef RTEMSCFG_MONITOR_MEM
    printf("Memory Statistics:\n");
    printf("  Total memory: %llu bytes\n", (unsigned long long)snapshot.mem.bytes_total);
    printf("  Used memory: %llu bytes\n", (unsigned long long)snapshot.mem.bytes_used);
    printf("  Free memory: %llu bytes\n", (unsigned long long)snapshot.mem.bytes_free);
    printf("  Largest free block: %llu bytes\n", (unsigned long long)snapshot.mem.bytes_high_water);
#endif
}

static rtems_task Init(rtems_task_argument ignored)
{
    rtems_id tid1, tid2, tid3, tid4, tid5;
    rtems_status_code sc;

    rtems_print_printer_fprintf_putc(&rtems_test_printer);
    TEST_BEGIN();

#ifdef RTEMSCFG_MONITOR_MEM
    printf("Memory Monitor Detailed Test started!\n");
    
    // 初始化监控系统
    rtems_monitor_initialize();
    printf("Monitor system initialized\n");
    
    // 初始快照
    print_monitor_snapshot("Initial");
    
    // 创建大内存任务
    sc = rtems_task_create(
        rtems_build_name('L','A','R','G'),
        1,  // 最高优先级
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid1
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid1, large_memory_task, 1);
        printf("Large memory task created and started\n");
    }
    
    // 创建中等内存任务
    sc = rtems_task_create(
        rtems_build_name('M','E','D','I'),
        2,  // 中等优先级
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid2
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid2, medium_memory_task, 1);
        printf("Medium memory task created and started\n");
    }
    
    // 创建小内存任务
    sc = rtems_task_create(
        rtems_build_name('S','M','A','L'),
        3,  // 低优先级
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid3
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid3, small_memory_task, 1);
        printf("Small memory task created and started\n");
    }
    
    // 等待一段时间让任务运行
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(200));
    
    // 采样并打印快照
    print_monitor_snapshot("After First Batch");
    
    // 创建更多任务
    sc = rtems_task_create(
        rtems_build_name('L','A','R','2'),
        1,  // 最高优先级
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid4
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid4, large_memory_task, 2);
        printf("Large memory task 2 created and started\n");
    }
    
    sc = rtems_task_create(
        rtems_build_name('S','M','A','2'),
        3,  // 低优先级
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &tid5
    );
    if (sc == RTEMS_SUCCESSFUL) {
        rtems_task_start(tid5, small_memory_task, 2);
        printf("Small memory task 2 created and started\n");
    }
    
    // 等待任务完成
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(500));
    
    // 最终采样
    print_monitor_snapshot("Final");
    
    // 打印简单监控信息
    printf("\n=== Simple Monitor Output ===\n");
    rtems_monitor_print_line();
    
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
#define CONFIGURE_MAXIMUM_TASKS            6
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
