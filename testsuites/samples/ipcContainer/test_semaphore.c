// 测试IPC容器中RTEMS信号量的隔离功能
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems.h>
#include <tmacros.h>
#include <errno.h>
#include <string.h>
#include <rtems/score/container.h>
#include <rtems/score/ipcContainer.h>
#include <rtems/score/threadimpl.h>
#include <stdio.h>
#include <stdlib.h>

const char rtems_test_name[] = "IPC CONTAINER RTEMS SEMAPHORE TEST";

// 全局变量
static IpcContainer *test_ipc_container1;
static IpcContainer *test_ipc_container2;
static rtems_id container1_task_id;
static rtems_id container2_task_id;
static rtems_id cross_container_task_id;

// 测试结果统计
static volatile int container1_semaphore_operations = 0;
static volatile int container2_semaphore_operations = 0;
static volatile int cross_container_access_attempts = 0;
static volatile int cross_container_access_failures = 0;
static volatile int test_completed = 0;

// 信号量名称定义
static rtems_name container1_sem_name = rtems_build_name('C', '1', 'S', 'M');
// static rtems_name container2_sem_name = rtems_build_name('C', '2', 'S', 'M');
// static rtems_name shared_sem_name = rtems_build_name('S', 'H', 'R', 'D');

// 信号量ID存储
static rtems_id container1_semaphore_id = RTEMS_ID_NONE;
static rtems_id container2_semaphore_id = RTEMS_ID_NONE;

// 创建测试IPC容器
static bool create_test_ipc_containers(void)
{
    printf("=== 创建测试IPC容器 ===\n");

    // 创建第一个IPC容器
    test_ipc_container1 = rtems_ipc_container_create();
    if (!test_ipc_container1) {
        printf("ERROR: 创建第一个IPC容器失败\n");
        return false;
    }
    printf("SUCCESS: 创建第一个IPC容器成功，ID: %d\n", test_ipc_container1->containerID);

    // 创建第二个IPC容器
    test_ipc_container2 = rtems_ipc_container_create();
    if (!test_ipc_container2) {
        printf("ERROR: 创建第二个IPC容器失败\n");
        return false;
    }
    printf("SUCCESS: 创建第二个IPC容器成功，ID: %d\n", test_ipc_container2->containerID);

    printf("SUCCESS: 两个IPC容器创建完成\n\n");
    return true;
}

// 容器2中的信号量测试任务
static rtems_task container1_semaphore_task(rtems_task_argument arg)
{
    rtems_status_code sc;
    
    printf("INFO: 容器2信号量测试任务启动\n");
    
    // 切换到容器2
    rtems_interrupt_level level;
    rtems_interrupt_disable(level);
    Thread_Control *executing = _Thread_Get_executing();
    IpcContainer *old_container = executing->container->ipcContainer;
    rtems_interrupt_enable(level);
    
    rtems_ipc_container_move_task(old_container, test_ipc_container1, executing);
    printf("INFO: 任务 0x%08x 切换到IPC容器 %d\n", 
           (unsigned int)executing->Object.id, test_ipc_container1->containerID);
    
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));

    printf("INFO: 在IPC容器2中创建信号量...\n");

    // 在容器2中创建信号量
    sc = rtems_semaphore_create(
        container1_sem_name,
        1,                                          
        RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY,    
        0,                                        
        &container1_semaphore_id
    );

    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 容器2信号量创建失败: %s\n", rtems_status_text(sc));
        goto exit;
    }
    printf("SUCCESS: 容器2信号量创建成功，ID: 0x%08x\n", (unsigned int)container1_semaphore_id);

    // 在容器2中进行信号量操作
    printf("INFO: 容器2开始信号量操作...\n");
    for (int i = 0; i < 3; i++) {
        // 获取信号量
        sc = rtems_semaphore_obtain(
            container1_semaphore_id,
            RTEMS_WAIT,
            RTEMS_NO_TIMEOUT
        );

        if (sc == RTEMS_SUCCESSFUL) {
            printf("SUCCESS: 容器2第%d次获取信号量成功\n", i + 1);
            container1_semaphore_operations++;
            
            // 模拟临界区操作
            rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(50));
            
            // 释放信号量
            sc = rtems_semaphore_release(container1_semaphore_id);
            if (sc == RTEMS_SUCCESSFUL) {
                printf("SUCCESS: 容器2第%d次释放信号量成功\n", i + 1);
                container1_semaphore_operations++;
            } else {
                printf("ERROR: 容器2第%d次释放信号量失败: %s\n", i + 1, rtems_status_text(sc));
            }
        } else {
            printf("ERROR: 容器2第%d次获取信号量失败: %s\n", i + 1, rtems_status_text(sc));
        }
        
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));
    }

    printf("INFO: 容器2信号量操作完成，总操作次数: %d\n", container1_semaphore_operations);

exit:
    printf("INFO: 容器2信号量测试任务结束\n");
    rtems_task_delete(RTEMS_SELF);
}

// 容器3中的信号量测试任务
static rtems_task container2_semaphore_task(rtems_task_argument arg)
{
    rtems_status_code sc;
    
    printf("INFO: 容器3信号量测试任务启动\n");
    
    // 切换到容器3
    rtems_interrupt_level level;
    rtems_interrupt_disable(level);
    Thread_Control *executing = _Thread_Get_executing();
    IpcContainer *old_container = executing->container->ipcContainer;
    rtems_interrupt_enable(level);
    
    rtems_ipc_container_move_task(old_container, test_ipc_container2, executing);
    printf("INFO: 任务 0x%08x 切换到IPC容器 %d\n", 
           (unsigned int)executing->Object.id, test_ipc_container2->containerID);
    
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(200));

    printf("INFO: 在IPC容器3中创建信号量...\n");

    // 在容器3中创建同名信号量
    sc = rtems_semaphore_create(
        container1_sem_name,  // 故意使用相同的名称
        2,                                          // 初始计数为2
        RTEMS_COUNTING_SEMAPHORE | RTEMS_PRIORITY,  // 计数信号量
        0,
        &container2_semaphore_id
    );

    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 容器3信号量创建失败: %s\n", rtems_status_text(sc));
        goto exit;
    }
    printf("SUCCESS: 容器3信号量创建成功，ID: 0x%08x（使用相同名称但在不同容器中）\n", 
           (unsigned int)container2_semaphore_id);

    // 在容器3中进行信号量操作
    printf("INFO: 容器3开始信号量操作...\n");
    for (int i = 0; i < 4; i++) {
        // 获取信号量
        sc = rtems_semaphore_obtain(
            container2_semaphore_id,
            RTEMS_WAIT,
            RTEMS_MILLISECONDS_TO_TICKS(1000)
        );

        if (sc == RTEMS_SUCCESSFUL) {
            printf("SUCCESS: 容器3第%d次获取信号量成功\n", i + 1);
            container2_semaphore_operations++;
            
            // 模拟临界区操作
            rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(30));
            
            // 总是释放信号量，确保操作对称
            sc = rtems_semaphore_release(container2_semaphore_id);
            if (sc == RTEMS_SUCCESSFUL) {
                printf("SUCCESS: 容器3第%d次释放信号量成功\n", i + 1);
                container2_semaphore_operations++;
            } else {
                printf("ERROR: 容器3第%d次释放信号量失败: %s\n", i + 1, rtems_status_text(sc));
            }
        } else {
            printf("ERROR: 容器3第%d次获取信号量失败: %s\n", i + 1, rtems_status_text(sc));
        }
        
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(80));
    }

    printf("INFO: 容器3信号量操作完成，总操作次数: %d\n", container2_semaphore_operations);

exit:
    printf("INFO: 容器3信号量测试任务结束\n");
    rtems_task_delete(RTEMS_SELF);
}

// 跨容器访问测试任务
static rtems_task cross_container_access_task(rtems_task_argument arg)
{
    rtems_status_code sc;
    rtems_id found_sem_id;
    
    printf("INFO: 跨容器访问测试任务启动\n");
    
    // 首先切换到容器3
    rtems_interrupt_level level;
    rtems_interrupt_disable(level);
    Thread_Control *executing = _Thread_Get_executing();
    IpcContainer *old_container = executing->container->ipcContainer;
    rtems_interrupt_enable(level);
    
    rtems_ipc_container_move_task(old_container, test_ipc_container2, executing);
    printf("INFO: 任务 0x%08x 切换到IPC容器 %d\n", 
           (unsigned int)executing->Object.id, test_ipc_container2->containerID);
    
    // 等待其他任务创建信号量
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(500));

    printf("INFO: 尝试从容器3访问容器2的信号量...\n");
    
    // 尝试通过ID访问容器2的信号量（应该失败）
    if (container1_semaphore_id != RTEMS_ID_NONE) {
        cross_container_access_attempts++;
        
        printf("INFO: 尝试获取容器2信号量 (ID: 0x%08x)\n", (unsigned int)container1_semaphore_id);
        sc = rtems_semaphore_obtain(
            container1_semaphore_id,
            RTEMS_NO_WAIT,
            0
        );

        if (sc == RTEMS_SUCCESSFUL) {
            printf("WARNING: 跨容器信号量访问成功（这不应该发生！）\n");
            rtems_semaphore_release(container1_semaphore_id);
        } else {
            printf("SUCCESS: 跨容器信号量访问失败（符合预期）: %s\n", rtems_status_text(sc));
            cross_container_access_failures++;
        }
    }

    // 尝试通过名称查找其他容器的信号量
    cross_container_access_attempts++;
    sc = rtems_semaphore_ident(
        container1_sem_name,
        RTEMS_SEARCH_ALL_NODES,
        &found_sem_id
    );

    if (sc == RTEMS_SUCCESSFUL) {
        printf("INFO: 在容器3中找到同名信号量 ID: 0x%08x\n", (unsigned int)found_sem_id);
        if (found_sem_id == container2_semaphore_id) {
            printf("SUCCESS: 找到的是容器3自己的信号量（符合预期）\n");
            cross_container_access_failures++;
        } else {
            printf("WARNING: 找到了不同容器的信号量（可能存在隔离问题）\n");
        }
    } else {
        printf("INFO: 在容器3中未找到指定名称的信号量: %s\n", rtems_status_text(sc));
        cross_container_access_failures++;
    }

    // 现在切换到容器2，尝试访问容器3的信号量
    printf("INFO: 切换到容器2，尝试访问容器3的资源...\n");
    
    rtems_interrupt_disable(level);
    executing = _Thread_Get_executing();
    old_container = executing->container->ipcContainer;
    rtems_interrupt_enable(level);
    
    rtems_ipc_container_move_task(old_container, test_ipc_container1, executing);
    printf("INFO: 任务 0x%08x 切换到IPC容器 %d\n", 
           (unsigned int)executing->Object.id, test_ipc_container1->containerID);
    
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));

    if (container2_semaphore_id != RTEMS_ID_NONE) {
        cross_container_access_attempts++;
        
        printf("INFO: 从容器2尝试访问容器3信号量 (ID: 0x%08x)\n", (unsigned int)container2_semaphore_id);
        sc = rtems_semaphore_obtain(
            container2_semaphore_id,
            RTEMS_NO_WAIT,
            0
        );

        if (sc == RTEMS_SUCCESSFUL) {
            printf("WARNING: 从容器2成功访问容器3信号量（这不应该发生！）\n");
            rtems_semaphore_release(container2_semaphore_id);
        } else {
            printf("SUCCESS: 从容器2访问容器3信号量失败（符合预期）: %s\n", rtems_status_text(sc));
            cross_container_access_failures++;
        }
    }

    printf("INFO: 跨容器访问测试完成\n");
    printf("INFO: 跨容器访问尝试次数: %d，失败次数: %d\n", 
           cross_container_access_attempts, cross_container_access_failures);

    rtems_task_delete(RTEMS_SELF);
}

// 创建和启动测试任务
static bool create_and_start_tasks(void)
{
    rtems_status_code sc;

    printf("=== 创建测试任务 ===\n");

    // 创建容器2信号量测试任务
    sc = rtems_task_create(
        rtems_build_name('C', '1', 'S', 'T'),
        1,
        RTEMS_MINIMUM_STACK_SIZE * 2,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &container1_task_id
    );

    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 容器2任务创建失败: %s\n", rtems_status_text(sc));
        return false;
    }

    // 创建容器3信号量测试任务
    sc = rtems_task_create(
        rtems_build_name('C', '2', 'S', 'T'),
        1,
        RTEMS_MINIMUM_STACK_SIZE * 2,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &container2_task_id
    );

    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 容器3任务创建失败: %s\n", rtems_status_text(sc));
        return false;
    }

    // 创建跨容器访问测试任务
    sc = rtems_task_create(
        rtems_build_name('C', 'R', 'S', 'T'),
        1,
        RTEMS_MINIMUM_STACK_SIZE * 2,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &cross_container_task_id
    );

    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 跨容器测试任务创建失败: %s\n", rtems_status_text(sc));
        return false;
    }

    // 启动任务
    sc = rtems_task_start(container1_task_id, container1_semaphore_task, 0);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 启动容器2任务失败: %s\n", rtems_status_text(sc));
        return false;
    }

    sc = rtems_task_start(container2_task_id, container2_semaphore_task, 0);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 启动容器3任务失败: %s\n", rtems_status_text(sc));
        return false;
    }

    sc = rtems_task_start(cross_container_task_id, cross_container_access_task, 0);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 启动跨容器测试任务失败: %s\n", rtems_status_text(sc));
        return false;
    }

    printf("SUCCESS: 所有测试任务创建并启动完成\n\n");
    return true;
}

// 等待任务完成
static void wait_for_tasks_completion(void)
{
    printf("=== 等待任务完成 ===\n");
    printf("INFO: 使用时间延迟等待任务完成\n");

    // 等待足够长的时间让任务完成
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(8000)); // 等待8秒

    // 监控信号量操作进度
    int wait_count = 0;
    while (wait_count < 15 && 
           (container1_semaphore_operations == 0 || 
            container2_semaphore_operations == 0 || 
            cross_container_access_attempts == 0)) {
        printf("INFO: 等待操作完成... (容器2操作:%d, 容器3操作:%d, 跨容器尝试:%d)\n",
               container1_semaphore_operations, container2_semaphore_operations, 
               cross_container_access_attempts);
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(500));
        wait_count++;
    }

    test_completed = 1;
    printf("SUCCESS: 任务监控完成\n\n");
}

// 打印测试结果
static void print_test_results(void)
{
    printf("=== 测试结果统计 ===\n");
    printf("容器2信号量操作次数: %d\n", container1_semaphore_operations);
    printf("容器3信号量操作次数: %d\n", container2_semaphore_operations);
    printf("跨容器访问尝试次数: %d\n", cross_container_access_attempts);
    printf("跨容器访问失败次数: %d\n", cross_container_access_failures);

    printf("\n=== 测试验证 ===\n");

    // 验证容器内操作
    if (container1_semaphore_operations >= 6 && container2_semaphore_operations >= 8) {
        printf(" PASS: 容器内信号量操作正常\n");
    } else {
        printf("FAIL: 容器内信号量操作不足（容器2: %d, 容器3: %d）\n", 
               container1_semaphore_operations, container2_semaphore_operations);
    }

    // 验证跨容器隔离
    if (cross_container_access_attempts > 0 && 
        cross_container_access_failures == cross_container_access_attempts) {
        printf(" PASS: 跨容器信号量访问完全被阻止，隔离性正确\n");
    } else if (cross_container_access_attempts > 0 && 
               cross_container_access_failures > 0) {
        printf("PARTIAL: 部分跨容器访问被阻止，隔离性部分有效\n");
        printf("    成功率: %.1f%%\n", 
               (float)cross_container_access_failures / cross_container_access_attempts * 100);
    } else {
        printf("FAIL: 跨容器信号量访问测试未执行或隔离失败\n");
    }

    // 验证容器状态
    printf("\n=== 容器状态验证 ===\n");
    if (test_ipc_container1) {
        printf("容器2状态: ID=%d, 引用计数=%d\n", 
               test_ipc_container1->containerID, test_ipc_container1->rc);
    }
    
    if (test_ipc_container2) {
        printf("容器3状态: ID=%d, 引用计数=%d\n", 
               test_ipc_container2->containerID, test_ipc_container2->rc);
    }

    // 总体测试结果
    printf("\n=== 总体测试结果 ===\n");
    if (container1_semaphore_operations >= 6 && 
        container2_semaphore_operations >= 8 &&
        cross_container_access_failures == cross_container_access_attempts &&
        cross_container_access_attempts > 0) {
        printf("测试结果: 全部通过 - IPC信号量隔离功能正常工作\n");
    } else {
        printf("测试结果: 部分通过 - 某些功能可能存在问题\n");
    }
}

// 清理测试资源
static void cleanup_test_resources(void)
{
    printf("\n=== 清理测试资源 ===\n");

    // 恢复到根容器
    Container *root_container = rtems_container_get_root();
    if (root_container && root_container->ipcContainer) {
        rtems_interrupt_level level;
        rtems_interrupt_disable(level);
        Thread_Control *executing = _Thread_Get_executing();
        if (executing && executing->container) {
            executing->container->ipcContainer = root_container->ipcContainer;
            root_container->ipcContainer->rc++;
        }
        rtems_interrupt_enable(level);
        printf("SUCCESS: 主任务恢复到根容器\n");
    }

    // 删除容器2信号量 - 需要在容器2的上下文中删除
    if (container1_semaphore_id != RTEMS_ID_NONE && test_ipc_container1) {
        // 暂时切换到容器2
        Thread_Control *executing = _Thread_Get_executing();
        IpcContainer *original_container = executing->container->ipcContainer;
        executing->container->ipcContainer = test_ipc_container1;
        
        rtems_status_code sc = rtems_semaphore_delete(container1_semaphore_id);
        if (sc == RTEMS_SUCCESSFUL) {
            printf("SUCCESS: 容器2信号量删除成功\n");
        } else {
            printf("WARNING: 容器2信号量删除失败: %s\n", rtems_status_text(sc));
        }
        
        // 恢复原容器
        executing->container->ipcContainer = original_container;
        container1_semaphore_id = RTEMS_ID_NONE;
    }

    // 删除容器3信号量 - 需要在容器3的上下文中删除
    if (container2_semaphore_id != RTEMS_ID_NONE && test_ipc_container2) {
        Thread_Control *executing = _Thread_Get_executing();
        IpcContainer *original_container = executing->container->ipcContainer;
        executing->container->ipcContainer = test_ipc_container2;
        
        rtems_status_code sc = rtems_semaphore_delete(container2_semaphore_id);
        if (sc == RTEMS_SUCCESSFUL) {
            printf("SUCCESS: 容器3信号量删除成功\n");
        } else {
            printf("WARNING: 容器3信号量删除失败: %s\n", rtems_status_text(sc));
        }
        
        executing->container->ipcContainer = original_container;
        container2_semaphore_id = RTEMS_ID_NONE;
    }

    // 删除IPC容器
    if (test_ipc_container1) {
        rtems_ipc_container_delete(test_ipc_container1);
        printf("SUCCESS: IPC容器2删除完成\n");
    }

    if (test_ipc_container2) {
        rtems_ipc_container_delete(test_ipc_container2);
        printf("SUCCESS: IPC容器3删除完成\n");
    }

    printf("SUCCESS: 所有资源清理完成\n");
}

// 主测试任务
static rtems_task Init(rtems_task_argument ignored)
{
    rtems_print_printer_fprintf_putc(&rtems_test_printer);
    TEST_BEGIN();

    printf("开始IPC容器信号量隔离测试\n");
    printf("========================================\n");

    // 打印主任务线程地址
    rtems_interrupt_level level;
    rtems_interrupt_disable(level);
    Thread_Control *current_thread = _Thread_Get_executing();
    rtems_interrupt_enable(level);
    printf("INFO: 主任务线程地址: %p, ID: 0x%08x\n", 
           (void *)current_thread, (unsigned int)current_thread->Object.id);

    // 执行测试步骤
    if (!create_test_ipc_containers()) {
        printf("ERROR: IPC容器创建失败\n");
        goto exit_fail;
    }

    if (!create_and_start_tasks()) {
        printf("ERROR: 测试任务创建失败\n");
        goto exit_fail;
    }

    wait_for_tasks_completion();
    print_test_results();
    cleanup_test_resources();

    printf("========================================\n");
    printf("IPC容器信号量隔离测试完成\n");

    TEST_END();
    rtems_test_exit(0);
    return;

exit_fail:
    cleanup_test_resources();
    printf("========================================\n");
    printf("IPC容器信号量隔离测试失败\n");
    TEST_END();
    rtems_test_exit(1);
}

/* 配置参数 */
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER

/* 任务配置 */
#define CONFIGURE_MAXIMUM_TASKS 16

/* RTEMS信号量配置 */
#define CONFIGURE_MAXIMUM_SEMAPHORES 25

/* 内存配置 */
#define CONFIGURE_EXTRA_TASK_STACKS (5 * RTEMS_MINIMUM_STACK_SIZE)

/* 初始化配置 */
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_INIT_TASK_STACK_SIZE (RTEMS_MINIMUM_STACK_SIZE * 8)
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_INIT
#include <rtems/confdefs.h>