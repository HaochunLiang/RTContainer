// 测试IPC容器中RTEMS消息队列的正常使用功能
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

const char rtems_test_name[] = "IPC CONTAINER MESSAGE QUEUE ISOLATION TEST";

// 全局变量
static IpcContainer *test_ipc_container1;
static IpcContainer *test_ipc_container2;
static rtems_id container1_sender_id;
static rtems_id container1_receiver_id;
static rtems_id container2_sender_id;
static rtems_id container2_receiver_id;
static rtems_id cross_container_task_id;

// 队列ID
static rtems_id queue1_id = RTEMS_ID_NONE;
static rtems_id queue2_id = RTEMS_ID_NONE;

// 测试结果统计
static volatile int container1_messages_sent = 0;
static volatile int container1_messages_received = 0;
static volatile int container2_messages_sent = 0;
static volatile int container2_messages_received = 0;
static volatile int cross_container_access_attempts = 0;
static volatile int cross_container_access_failures = 0;
static volatile int test_completed = 0;

// 测试消息结构
typedef struct
{
    int seq_num;
    char content[32];
} test_message_t;

// 创建测试IPC容器
static bool create_test_ipc_containers(void)
{
    printf("=== 创建测试IPC容器 ===\n");

    // 创建容器1
    test_ipc_container1 = rtems_ipc_container_create();
    if (!test_ipc_container1)
    {
        printf("ERROR: 创建容器1失败\n");
        return false;
    }
    printf("SUCCESS: 创建容器1成功，ID: %d\n", test_ipc_container1->containerID);

    // 创建容器2
    test_ipc_container2 = rtems_ipc_container_create();
    if (!test_ipc_container2)
    {
        printf("ERROR: 创建容器2失败\n");
        return false;
    }
    printf("SUCCESS: 创建容器2成功，ID: %d\n\n", test_ipc_container2->containerID);

    return true;
}

// 容器1发送任务
static rtems_task container1_sender_task(rtems_task_argument arg)
{
    rtems_status_code sc;
    test_message_t msg;

    printf("INFO: 容器1发送任务启动\n");
    
    // 切换到容器1
    rtems_interrupt_level level;
    rtems_interrupt_disable(level);
    Thread_Control *executing = _Thread_Get_executing();
    IpcContainer *old_container = executing->container->ipcContainer;
    rtems_interrupt_enable(level);
    
    rtems_ipc_container_move_task(old_container, test_ipc_container1, executing);
    printf("INFO: 任务 0x%08x 切换到容器 %d\n", 
           (unsigned int)executing->Object.id, test_ipc_container1->containerID);
    
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));

    printf("INFO: 在容器1中创建消息队列...\n");
    sc = rtems_message_queue_create(
        rtems_build_name('T', 'S', 'T', 'Q'),  // 同名
        10,
        sizeof(test_message_t),
        RTEMS_DEFAULT_ATTRIBUTES,
        &queue1_id);

    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 容器1队列创建失败: %s\n", rtems_status_text(sc));
        goto exit;
    }
    printf("SUCCESS: 容器1队列创建成功，ID: 0x%08x\n", (unsigned int)queue1_id);

    // 发送消息
    for (int i = 1; i <= 5; i++) {
        msg.seq_num = i;
        snprintf(msg.content, sizeof(msg.content), "容器1消息%d", i);

        sc = rtems_message_queue_send(queue1_id, &msg, sizeof(msg));
        if (sc == RTEMS_SUCCESSFUL) {
            container1_messages_sent++;
            printf("SUCCESS: 容器1发送消息 %d\n", i);
        } else {
            printf("ERROR: 容器1发送消息 %d 失败: %s\n", i, rtems_status_text(sc));
        }
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(200));
    }

    printf("INFO: 容器1发送完成，总共: %d\n", container1_messages_sent);

exit:
    rtems_task_delete(RTEMS_SELF);
}

// 容器1接收任务
static rtems_task container1_receiver_task(rtems_task_argument arg)
{
    rtems_status_code sc;
    test_message_t msg;
    size_t msg_size;
    int timeout_count = 0;

    printf("INFO: 容器1接收任务启动\n");
    
    // 切换到容器1
    rtems_interrupt_level level;
    rtems_interrupt_disable(level);
    Thread_Control *executing = _Thread_Get_executing();
    IpcContainer *old_container = executing->container->ipcContainer;
    rtems_interrupt_enable(level);
    
    rtems_ipc_container_move_task(old_container, test_ipc_container1, executing);
    printf("INFO: 任务 0x%08x 切换到容器 %d\n", 
           (unsigned int)executing->Object.id, test_ipc_container1->containerID);
    
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(300));

    printf("INFO: 容器1开始接收消息，队列ID: 0x%08x\n", (unsigned int)queue1_id);
    while (timeout_count < 10 && container1_messages_received < 5) {
        sc = rtems_message_queue_receive(
            queue1_id,
            &msg,
            &msg_size,
            RTEMS_DEFAULT_OPTIONS,
            RTEMS_MILLISECONDS_TO_TICKS(1000));

        if (sc == RTEMS_SUCCESSFUL) {
            container1_messages_received++;
            printf("SUCCESS: 容器1接收消息 %d: %s\n", msg.seq_num, msg.content);
            timeout_count = 0;
        } else if (sc == RTEMS_TIMEOUT) {
            timeout_count++;
        } else {
            printf("ERROR: 容器1接收失败: %s\n", rtems_status_text(sc));
            break;
        }
    }

    printf("INFO: 容器1接收完成，总共: %d\n", container1_messages_received);
    rtems_task_delete(RTEMS_SELF);
}

// 容器2发送任务
static rtems_task container2_sender_task(rtems_task_argument arg)
{
    rtems_status_code sc;
    test_message_t msg;

    printf("INFO: 容器2发送任务启动\n");
    
    // 切换到容器2
    rtems_interrupt_level level;
    rtems_interrupt_disable(level);
    Thread_Control *executing = _Thread_Get_executing();
    IpcContainer *old_container = executing->container->ipcContainer;
    rtems_interrupt_enable(level);
    
    rtems_ipc_container_move_task(old_container, test_ipc_container2, executing);
    printf("INFO: 任务 0x%08x 切换到容器 %d\n", 
           (unsigned int)executing->Object.id, test_ipc_container2->containerID);
    
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(200));

    printf("INFO: 在容器2中创建消息队列(同名)...\n");
    sc = rtems_message_queue_create(
        rtems_build_name('T', 'S', 'T', 'Q'),  // 故意同名
        10,
        sizeof(test_message_t),
        RTEMS_DEFAULT_ATTRIBUTES,
        &queue2_id);

    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 容器2队列创建失败: %s\n", rtems_status_text(sc));
        goto exit;
    }
    printf("SUCCESS: 容器2队列创建成功，ID: 0x%08x\n", (unsigned int)queue2_id);

    // 发送消息
    for (int i = 1; i <= 5; i++) {
        msg.seq_num = i;
        snprintf(msg.content, sizeof(msg.content), "容器2消息%d", i);

        sc = rtems_message_queue_send(queue2_id, &msg, sizeof(msg));
        if (sc == RTEMS_SUCCESSFUL) {
            container2_messages_sent++;
            printf("SUCCESS: 容器2发送消息 %d\n", i);
        } else {
            printf("ERROR: 容器2发送消息 %d 失败: %s\n", i, rtems_status_text(sc));
        }
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(200));
    }

    printf("INFO: 容器2发送完成，总共: %d\n", container2_messages_sent);

exit:
    rtems_task_delete(RTEMS_SELF);
}

// 容器2接收任务
static rtems_task container2_receiver_task(rtems_task_argument arg)
{
    rtems_status_code sc;
    test_message_t msg;
    size_t msg_size;
    int timeout_count = 0;

    printf("INFO: 容器2接收任务启动\n");
    
    // 切换到容器2
    rtems_interrupt_level level;
    rtems_interrupt_disable(level);
    Thread_Control *executing = _Thread_Get_executing();
    IpcContainer *old_container = executing->container->ipcContainer;
    rtems_interrupt_enable(level);
    
    rtems_ipc_container_move_task(old_container, test_ipc_container2, executing);
    printf("INFO: 任务 0x%08x 切换到容器 %d\n", 
           (unsigned int)executing->Object.id, test_ipc_container2->containerID);
    
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(400));

    printf("INFO: 容器2开始接收消息，队列ID: 0x%08x\n", (unsigned int)queue2_id);
    while (timeout_count < 10 && container2_messages_received < 5) {
        sc = rtems_message_queue_receive(
            queue2_id,
            &msg,
            &msg_size,
            RTEMS_DEFAULT_OPTIONS,
            RTEMS_MILLISECONDS_TO_TICKS(1000));

        if (sc == RTEMS_SUCCESSFUL) {
            container2_messages_received++;
            printf("SUCCESS: 容器2接收消息 %d: %s\n", msg.seq_num, msg.content);
            timeout_count = 0;
        } else if (sc == RTEMS_TIMEOUT) {
            timeout_count++;
        } else {
            printf("ERROR: 容器2接收失败: %s\n", rtems_status_text(sc));
            break;
        }
    }

    printf("INFO: 容器2接收完成，总共: %d\n", container2_messages_received);
    rtems_task_delete(RTEMS_SELF);
}

// 跨容器访问测试任务
static rtems_task cross_container_access_task(rtems_task_argument arg)
{
    rtems_status_code sc;
    rtems_id found_id;
    test_message_t msg;
    size_t msg_size;

    printf("INFO: 跨容器访问测试任务启动\n");
    
    // 切换到容器2
    rtems_interrupt_level level;
    rtems_interrupt_disable(level);
    Thread_Control *executing = _Thread_Get_executing();
    IpcContainer *old_container = executing->container->ipcContainer;
    rtems_interrupt_enable(level);
    
    rtems_ipc_container_move_task(old_container, test_ipc_container2, executing);
    printf("INFO: 任务 0x%08x 切换到容器 %d\n", 
           (unsigned int)executing->Object.id, test_ipc_container2->containerID);
    
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(500));

    // 测试1: 从容器2访问容器1队列(通过ID发送)
    printf("INFO: 测试从容器2访问容器1队列(ID: 0x%08x)\n", (unsigned int)queue1_id);
    cross_container_access_attempts++;
    msg.seq_num = 999;
    snprintf(msg.content, sizeof(msg.content), "跨容器消息");
    
    sc = rtems_message_queue_send(queue1_id, &msg, sizeof(msg));
    if (sc != RTEMS_SUCCESSFUL) {
        printf("SUCCESS: 跨容器发送失败(符合预期): %s\n", rtems_status_text(sc));
        cross_container_access_failures++;
    } else {
        printf("WARNING: 跨容器发送成功(不应该发生!)\n");
    }

    // 测试2: 从容器2尝试接收容器1的消息
    cross_container_access_attempts++;
    sc = rtems_message_queue_receive(queue1_id, &msg, &msg_size, RTEMS_NO_WAIT, 0);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("SUCCESS: 跨容器接收失败(符合预期): %s\n", rtems_status_text(sc));
        cross_container_access_failures++;
    } else {
        printf("WARNING: 跨容器接收成功(不应该发生!)\n");
    }

    // 测试3: 通过名称查找队列
    cross_container_access_attempts++;
    sc = rtems_message_queue_ident(
        rtems_build_name('T', 'S', 'T', 'Q'),
        RTEMS_SEARCH_ALL_NODES,
        &found_id);

    if (sc == RTEMS_SUCCESSFUL) {
        if (found_id == queue2_id) {
            printf("SUCCESS: 找到容器2自己的队列(符合预期)\n");
            cross_container_access_failures++;
        } else if (found_id == queue1_id) {
            printf("WARNING: 找到了容器1的队列(隔离失败!)\n");
        }
    } else {
        printf("INFO: 未找到队列: %s\n", rtems_status_text(sc));
        cross_container_access_failures++;
    }

    // 测试4: 反向测试(容器1→容器2)
    printf("INFO: 切换到容器1测试反向访问\n");
    
    rtems_interrupt_disable(level);
    executing = _Thread_Get_executing();
    old_container = executing->container->ipcContainer;
    rtems_interrupt_enable(level);
    
    rtems_ipc_container_move_task(old_container, test_ipc_container1, executing);
    printf("INFO: 任务 0x%08x 切换到容器 %d\n", 
           (unsigned int)executing->Object.id, test_ipc_container1->containerID);
    
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));

    cross_container_access_attempts++;
    sc = rtems_message_queue_send(queue2_id, &msg, sizeof(msg));
    if (sc != RTEMS_SUCCESSFUL) {
        printf("SUCCESS: 从容器1访问容器2失败(符合预期)\n");
        cross_container_access_failures++;
    } else {
        printf("WARNING: 从容器1访问容器2成功(不应该发生!)\n");
    }

    printf("INFO: 跨容器访问测试完成 (尝试: %d, 失败: %d)\n",
           cross_container_access_attempts, cross_container_access_failures);
    rtems_task_delete(RTEMS_SELF);
}

// 创建和启动测试任务
static bool create_and_start_tasks(void)
{
    rtems_status_code sc;

    printf("=== 创建测试任务 ===\n");

    // 创建容器1发送任务
    sc = rtems_task_create(
        rtems_build_name('C', '1', 'S', 'D'),
        1,
        RTEMS_MINIMUM_STACK_SIZE * 4,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &container1_sender_id);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 容器1发送任务创建失败: %s\n", rtems_status_text(sc));
        return false;
    }

    // 创建容器1接收任务
    sc = rtems_task_create(
        rtems_build_name('C', '1', 'R', 'V'),
        1,
        RTEMS_MINIMUM_STACK_SIZE * 4,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &container1_receiver_id);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 容器1接收任务创建失败: %s\n", rtems_status_text(sc));
        return false;
    }

    // 创建容器2发送任务
    sc = rtems_task_create(
        rtems_build_name('C', '2', 'S', 'D'),
        1,
        RTEMS_MINIMUM_STACK_SIZE * 4,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &container2_sender_id);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 容器2发送任务创建失败: %s\n", rtems_status_text(sc));
        return false;
    }

    // 创建容器2接收任务
    sc = rtems_task_create(
        rtems_build_name('C', '2', 'R', 'V'),
        1,
        RTEMS_MINIMUM_STACK_SIZE * 4,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &container2_receiver_id);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 容器2接收任务创建失败: %s\n", rtems_status_text(sc));
        return false;
    }

    // 创建跨容器测试任务
    sc = rtems_task_create(
        rtems_build_name('C', 'R', 'S', 'S'),
        1,
        RTEMS_MINIMUM_STACK_SIZE * 4,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &cross_container_task_id);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 跨容器测试任务创建失败: %s\n", rtems_status_text(sc));
        return false;
    }

    // 启动所有任务
    rtems_task_start(container1_sender_id, container1_sender_task, 0);
    rtems_task_start(container1_receiver_id, container1_receiver_task, 0);
    rtems_task_start(container2_sender_id, container2_sender_task, 0);
    rtems_task_start(container2_receiver_id, container2_receiver_task, 0);
    rtems_task_start(cross_container_task_id, cross_container_access_task, 0);

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

    // 监控消息处理进度
    int wait_count = 0;
    while (wait_count < 15 && 
           (container1_messages_sent == 0 || 
            container2_messages_sent == 0 || 
            cross_container_access_attempts == 0)) {
        printf("INFO: 等待操作完成... (容器1发送:%d 接收:%d, 容器2发送:%d 接收:%d, 跨容器:%d)\n",
               container1_messages_sent, container1_messages_received,
               container2_messages_sent, container2_messages_received,
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
    printf("容器1: 发送 %d, 接收 %d\n", 
           container1_messages_sent, container1_messages_received);
    printf("容器2: 发送 %d, 接收 %d\n", 
           container2_messages_sent, container2_messages_received);
    printf("跨容器访问尝试: %d, 失败: %d\n",
           cross_container_access_attempts, cross_container_access_failures);

    printf("\n=== 测试验证 ===\n");

    // 验证容器内通信
    bool container1_ok = (container1_messages_sent >= 5 && 
                          container1_messages_received == container1_messages_sent);
    bool container2_ok = (container2_messages_sent >= 5 && 
                          container2_messages_received == container2_messages_sent);

    if (container1_ok && container2_ok) {
        printf(" PASS: 两个容器内部消息队列通信正常\n");
    } else {
        printf(" FAIL: 容器内部通信失败 (容器1: %d/%d, 容器2: %d/%d)\n",
               container1_messages_received, container1_messages_sent,
               container2_messages_received, container2_messages_sent);
    }

    // 验证隔离性
    if (cross_container_access_attempts > 0 &&
        cross_container_access_failures == cross_container_access_attempts) {
        printf(" PASS: 跨容器消息队列访问完全被阻止，隔离性正确\n");
    } else if (cross_container_access_attempts > 0 && 
               cross_container_access_failures > 0) {
        printf("⚠️  PARTIAL: 部分跨容器访问被阻止 (%.1f%%)\n",
               (float)cross_container_access_failures / cross_container_access_attempts * 100);
    } else {
        printf(" FAIL: 跨容器隔离性验证失败\n");
    }

    // 验证同名队列隔离
    if (queue1_id != RTEMS_ID_NONE && queue2_id != RTEMS_ID_NONE) {
        printf(" PASS: 同名消息队列在不同容器中独立存在\n");
        printf("   - 容器1队列ID: 0x%08x\n", (unsigned int)queue1_id);
        printf("   - 容器2队列ID: 0x%08x\n", (unsigned int)queue2_id);
    }

    // 容器状态
    printf("\n=== 容器状态 ===\n");
    if (test_ipc_container1) {
        printf("容器1: ID=%d, 引用计数=%d\n", 
               test_ipc_container1->containerID, test_ipc_container1->rc);
    }
    if (test_ipc_container2) {
        printf("容器2: ID=%d, 引用计数=%d\n", 
               test_ipc_container2->containerID, test_ipc_container2->rc);
    }

    // 总体结果
    printf("\n=== 总体测试结果 ===\n");
    if (container1_ok && container2_ok &&
        cross_container_access_failures == cross_container_access_attempts &&
        cross_container_access_attempts > 0) {
        printf(" 测试通过 - IPC消息队列隔离功能正常工作\n");
    } else {
        printf(" 测试失败 - 某些功能存在问题\n");
    }
}

static void cleanup_test_resources(void)
{
    printf("\n=== 清理测试资源 ===\n");

    Thread_Control *current = _Thread_Get_executing();
    Container *root = rtems_container_get_root();
    
    // 删除容器1的队列
    if (queue1_id != RTEMS_ID_NONE && test_ipc_container1) {
        current->container->ipcContainer = test_ipc_container1;
        rtems_status_code sc = rtems_message_queue_delete(queue1_id);
        if (sc == RTEMS_SUCCESSFUL) {
            printf("SUCCESS: 容器1队列删除成功\n");
        } else {
            printf("WARNING: 容器1队列删除失败: %s\n", rtems_status_text(sc));
        }
        queue1_id = RTEMS_ID_NONE;
    }

    // 删除容器2的队列
    if (queue2_id != RTEMS_ID_NONE && test_ipc_container2) {
        current->container->ipcContainer = test_ipc_container2;
        rtems_status_code sc = rtems_message_queue_delete(queue2_id);
        if (sc == RTEMS_SUCCESSFUL) {
            printf("SUCCESS: 容器2队列删除成功\n");
        } else {
            printf("WARNING: 容器2队列删除失败: %s\n", rtems_status_text(sc));
        }
        queue2_id = RTEMS_ID_NONE;
    }

    // 恢复到根容器
    if (root && root->ipcContainer) {
        current->container->ipcContainer = root->ipcContainer;
        root->ipcContainer->rc++;
        printf("SUCCESS: 恢复到根容器\n");
    }

    // 删除IPC容器
    if (test_ipc_container1) {
        rtems_ipc_container_delete(test_ipc_container1);
        printf("SUCCESS: 容器1删除成功\n");
    }

    if (test_ipc_container2) {
        rtems_ipc_container_delete(test_ipc_container2);
        printf("SUCCESS: 容器2删除成功\n");
    }

    printf("SUCCESS: 所有资源清理完成\n");
}

// 主测试任务
static rtems_task Init(rtems_task_argument ignored)
{
    rtems_print_printer_fprintf_putc(&rtems_test_printer);
    TEST_BEGIN();

    // 打印主任务线程地址
    rtems_interrupt_level level;
    rtems_interrupt_disable(level);
    Thread_Control *current_thread = _Thread_Get_executing();
    rtems_interrupt_enable(level);
    printf("INFO: 主任务(Init)线程地址: %p\n", (void *)current_thread);
    printf("INFO: 主任务(Init)线程ID: 0x%08x\n", (unsigned int)current_thread->Object.id);

    printf("开始IPC容器消息队列隔离测试\n");
    printf("测试目标: 验证消息队列的跨容器隔离性\n");
    printf("========================================\n\n");

    // 创建测试IPC容器
    if (!create_test_ipc_containers())
    {
        printf("FATAL: 测试IPC容器创建失败\n");
        goto exit_fail;
    }

    // 创建和启动测试任务
    if (!create_and_start_tasks())
    {
        printf("FATAL: 测试任务创建失败\n");
        goto exit_fail;
    }

    // 等待任务完成
    wait_for_tasks_completion();

    // 打印测试结果
    print_test_results();

    // 清理资源
    cleanup_test_resources();

    printf("\n========================================\n");
    printf("IPC容器消息队列隔离测试完成\n");

    TEST_END();
    rtems_test_exit(0);

exit_fail:
    cleanup_test_resources();
    printf("\n========================================\n");
    printf("IPC容器消息队列隔离测试失败\n");
    TEST_END();
    rtems_test_exit(1);
}

/* 配置参数 */
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER

/* 任务配置 */
#define CONFIGURE_MAXIMUM_TASKS 16

/* RTEMS信号量配置 - 减少数量，因为主要测试消息队列 */
#define CONFIGURE_MAXIMUM_SEMAPHORES 5

/* RTEMS消息队列配置 */
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 10

/* 内存配置 */
#define CONFIGURE_MESSAGE_BUFFER_MEMORY \
    CONFIGURE_MESSAGE_BUFFERS_FOR_QUEUE(10, sizeof(test_message_t))

#define CONFIGURE_EXTRA_TASK_STACKS (10 * RTEMS_MINIMUM_STACK_SIZE)

/* 初始化配置 */
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_INIT_TASK_STACK_SIZE (RTEMS_MINIMUM_STACK_SIZE * 8)
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_INIT
#include <rtems/confdefs.h>