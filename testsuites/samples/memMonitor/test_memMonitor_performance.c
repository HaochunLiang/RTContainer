#include <stdio.h>
#include <rtems.h>
#include <rtems/score/monitor.h>
#include <rtems/libcsupport.h>
#include <tmacros.h>

const char rtems_test_name[] = "MEMORY MONITOR PERFORMANCE TEST";

#define PERFORMANCE_TEST_ITERATIONS 10
#define PERFORMANCE_TEST_TASKS 5

static rtems_task memory_performance_task(rtems_task_argument arg)
{
    int task_id = (int)arg;
    printf("Memory performance task %d started!\n", task_id);
    
    // 根据任务 ID 执行不同强度的内存操作
    void *ptrs[20];
    size_t base_size = 1024;
    
    // 分配内存
    for (int i = 0; i < 20; ++i) {
        size_t size = base_size * (1 + (task_id * 2) + (i % 3));
        ptrs[i] = malloc(size);
        if (ptrs[i]) {
            memset(ptrs[i], 0xAA + task_id, size);
        }
    }
    
    printf("Memory performance task %d allocated memory\n", task_id);
    
    // 等待一段时间
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));
    
    // 释放一半内存
    for (int i = 0; i < 10; ++i) {
        if (ptrs[i]) {
            free(ptrs[i]);
        }
    }
    
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));
    
    // 释放剩余内存
    for (int i = 10; i < 20; ++i) {
        if (ptrs[i]) {
            free(ptrs[i]);
        }
    }
    
    printf("Memory performance task %d completed!\n", task_id);
    rtems_task_exit();
}

static void measure_monitor_performance(void)
{
    printf("\n=== Monitor Performance Measurement ===\n");
    
    // 测量初始化性能
    uint32_t start_time = rtems_clock_get_ticks_since_boot();
    rtems_monitor_initialize();
    uint32_t end_time = rtems_clock_get_ticks_since_boot();
    printf("Monitor initialization took %llu ticks\n", 
           (unsigned long long)(end_time - start_time));
    
    // 测量采样性能
    start_time = rtems_clock_get_ticks_since_boot();
    for (int i = 0; i < PERFORMANCE_TEST_ITERATIONS; ++i) {
        rtems_monitor_sample();
    }
    end_time = rtems_clock_get_ticks_since_boot();
    printf("Monitor sampling (%d iterations) took %llu ticks\n", 
           PERFORMANCE_TEST_ITERATIONS, (unsigned long long)(end_time - start_time));
    
    // 测量获取快照性能
    RtemsMonitor snapshot;
    start_time = rtems_clock_get_ticks_since_boot();
    for (int i = 0; i < PERFORMANCE_TEST_ITERATIONS; ++i) {
        rtems_monitor_sample();  // 先采样
        rtems_monitor_get_snapshot(&snapshot);
    }
    end_time = rtems_clock_get_ticks_since_boot();
    printf("Monitor snapshot (%d iterations) took %llu ticks\n", 
           PERFORMANCE_TEST_ITERATIONS, (unsigned long long)(end_time - start_time));
    
    // 测量打印性能
    start_time = rtems_clock_get_ticks_since_boot();
    for (int i = 0; i < PERFORMANCE_TEST_ITERATIONS; ++i) {
        rtems_monitor_print_line();
    }
    end_time = rtems_clock_get_ticks_since_boot();
    printf("Monitor print line (%d iterations) took %llu ticks\n", 
           PERFORMANCE_TEST_ITERATIONS, (unsigned long long)(end_time - start_time));
}

static void stress_test_monitor(void)
{
    printf("\n=== Monitor Stress Test ===\n");
    
    rtems_id task_ids[PERFORMANCE_TEST_TASKS];
    rtems_status_code sc;
    
    // 创建多个任务进行压力测试
    for (int i = 0; i < PERFORMANCE_TEST_TASKS; ++i) {
        sc = rtems_task_create(
            rtems_build_name('M','E','M','P') + i,
            1 + (i % 3),  // 不同优先级
            RTEMS_MINIMUM_STACK_SIZE,
            RTEMS_DEFAULT_MODES,
            RTEMS_DEFAULT_ATTRIBUTES,
            &task_ids[i]
        );
        if (sc == RTEMS_SUCCESSFUL) {
            rtems_task_start(task_ids[i], memory_performance_task, i + 1);
            printf("Memory performance task %d created and started\n", i + 1);
        } else {
            printf("Failed to create memory performance task %d: %s\n", 
                   i + 1, rtems_status_text(sc));
        }
    }
    
    // 在任务运行期间进行监控
    for (int round = 0; round < 5; ++round) {
        printf("\n--- Monitoring Round %d ---\n", round + 1);
        
        // 采样
        rtems_monitor_sample();
        
        // 打印简单信息
        printf("Simple output: ");
        rtems_monitor_print_line();
        
        // 获取详细统计
        RtemsMemStats mem_stats;
        rtems_monitor_get_mem(&mem_stats);
        printf("Memory usage: %llu/%llu bytes (%.1f%%)\n", 
               (unsigned long long)mem_stats.bytes_used,
               (unsigned long long)mem_stats.bytes_total,
               (double)mem_stats.bytes_used * 100.0 / mem_stats.bytes_total);
        
        // 等待一段时间
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));
    }
    
    // 等待所有任务完成
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(1000));
    
    // 最终报告
    printf("\n=== Final Performance Report ===\n");
    rtems_monitor_sample();
    rtems_monitor_print_line();
}

static rtems_task Init(rtems_task_argument ignored)
{
    rtems_print_printer_fprintf_putc(&rtems_test_printer);
    TEST_BEGIN();

#ifdef RTEMSCFG_MONITOR_MEM
    printf("Memory Monitor Performance Test started!\n");
    
    // 性能测试
    measure_monitor_performance();
    
    // 压力测试
    stress_test_monitor();
    
    // 最终详细报告
    printf("\n=== Final Detailed Monitor Report ===\n");
    rtems_monitor_print_detailed_report(&rtems_test_printer);
    
#else
    printf("RTEMSCFG_MONITOR_MEM not defined\n");
#endif

    TEST_END();
    rtems_test_exit(0);
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_MAXIMUM_TASKS            (1 + PERFORMANCE_TEST_TASKS)
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
