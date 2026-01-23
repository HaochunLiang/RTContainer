/*
 * 网络容器端口复用隔离测试 (参考 LiteOS It_net_container_008)
 * 
 * 测试目标：
 *   验证不同网络容器可以同时绑定同一个端口号
 *   - 根容器绑定 UDP + TCP 到端口 8004
 *   - 子容器也绑定 UDP + TCP 到端口 8004
 *   - 验证两个容器都绑定成功（无 EADDRINUSE 错误）
 */

/* 解决宏定义冲突 */
#ifdef rtems_object_id_get_api
#undef rtems_object_id_get_api
#endif
#ifdef rtems_object_id_get_class  
#undef rtems_object_id_get_class
#endif
#ifdef rtems_object_id_get_index
#undef rtems_object_id_get_index  
#endif
#ifdef rtems_object_id_get_node
#undef rtems_object_id_get_node
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <rtems.h>
#include <rtems/rtems_bsdnet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <rtems/score/threadimpl.h>
#include <rtems/score/container.h>
#include <rtems/score/netContainer.h>
#include <rtems/score/wkspace.h>

#include <tmacros.h>

const char rtems_test_name[] = "NET CONTAINER 008 - PORT ISOLATION TEST";

/* ==================== 常量定义 ==================== */

#define LOOPBACK_IP     "127.0.0.1"
#define TEST_PORT       8004

/* ==================== 全局变量 ==================== */

static NetContainer *container_main = NULL;
static NetContainer *container_sub = NULL;
static rtems_id sub_task_id;
static rtems_id sync_sem, result_sem;

static volatile int main_udp_sock = -1;
static volatile int main_tcp_sock = -1;
static volatile int sub_result = -1;

/* ==================== 绑定 UDP + TCP ==================== */

static int bind_udp_tcp(const char *container_name, int *udp_sock, int *tcp_sock)
{
    struct sockaddr_in addr;
    int ret;
    
    printf("\n [%s] 尝试绑定 UDP + TCP 到端口 %d\n", container_name, TEST_PORT);
    
    // 1. 创建并绑定 UDP socket
    printf("  [%s] 创建 UDP socket...\n", container_name);
    *udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (*udp_sock < 0) {
        printf("   [%s] UDP socket() 失败: %s\n", container_name, strerror(errno));
        return 1;
    }
    printf("   [%s] UDP socket 创建成功 (fd=%d)\n", container_name, *udp_sock);
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(LOOPBACK_IP);
    addr.sin_port = htons(TEST_PORT);
    
    ret = bind(*udp_sock, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        printf("   [%s] UDP bind() 失败: %s\n", container_name, strerror(errno));
        if (errno == EADDRINUSE) {
            printf("       端口被占用！容器隔离可能失败\n");
        }
        close(*udp_sock);
        return 2;
    }
    printf("   [%s] UDP 绑定成功: %s:%d\n", container_name, LOOPBACK_IP, TEST_PORT);
    
    // 2. 创建并绑定 TCP socket
    printf("   [%s] 创建 TCP socket...\n", container_name);
    *tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*tcp_sock < 0) {
        printf("   [%s] TCP socket() 失败: %s\n", container_name, strerror(errno));
        close(*udp_sock);
        return 3;
    }
    printf("   [%s] TCP socket 创建成功 (fd=%d)\n", container_name, *tcp_sock);
    
    ret = bind(*tcp_sock, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        printf("   [%s] TCP bind() 失败: %s\n", container_name, strerror(errno));
        if (errno == EADDRINUSE) {
            printf("       端口被占用！容器隔离可能失败\n");
        }
        close(*udp_sock);
        close(*tcp_sock);
        return 4;
    }
    printf("   [%s] TCP 绑定成功: %s:%d\n", container_name, LOOPBACK_IP, TEST_PORT);
    
    printf("   [%s] UDP + TCP 都成功绑定到端口 %d\n", container_name, TEST_PORT);
    return 0;
}

/* ==================== 子容器测试任务 ==================== */

static rtems_task sub_container_task(rtems_task_argument arg)
{
    int udp_sock = -1, tcp_sock = -1;
    
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf(" 子容器任务开始\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    // 验证容器环境
    Thread_Control *executing = _Thread_Get_executing();
    if (!executing->container || !executing->container->netContainer) {
        printf(" 子任务未关联到网络容器\n");
        sub_result = 10;
        rtems_semaphore_release(result_sem);
        rtems_task_delete(RTEMS_SELF);
    }
    
    printf(" 子任务运行在网络容器 (ID=%d)\n", 
           executing->container->netContainer->containerID);
    
    // 等待根容器完成绑定
    printf(" 等待根容器完成绑定...\n");
    rtems_semaphore_obtain(sync_sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    
    // 在子容器中绑定相同的端口
    sub_result = bind_udp_tcp("子容器", &udp_sock, &tcp_sock);
    
    if (sub_result == 0) {
        printf("\n 子容器端口绑定成功！\n");
        printf("   根容器和子容器同时使用端口 %d，无冲突\n", TEST_PORT);
    }
    
    // 清理
    if (udp_sock >= 0) close(udp_sock);
    if (tcp_sock >= 0) close(tcp_sock);
    
    printf("\n 子容器任务完成\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    rtems_semaphore_release(result_sem);
    rtems_task_delete(RTEMS_SELF);
}

/* ==================== 主测试函数 ==================== */

static rtems_task Init(rtems_task_argument ignored)
{
    rtems_status_code sc;
    int main_result;
    
    TEST_BEGIN();
    
    // 初始化 BSD 网络栈
    printf(" 初始化 BSD 网络栈...\n");
    if (rtems_bsdnet_initialize_network() < 0) {
        printf(" 网络栈初始化失败\n");
        TEST_END();
        rtems_test_exit(1);
    }
    printf(" 网络栈初始化成功\n\n");
    
    // 创建同步信号量
    sc = rtems_semaphore_create(rtems_build_name('S', 'Y', 'N', 'C'), 0,
        RTEMS_SIMPLE_BINARY_SEMAPHORE | RTEMS_PRIORITY, 0, &sync_sem);
    sc |= rtems_semaphore_create(rtems_build_name('R', 'S', 'L', 'T'), 0,
        RTEMS_SIMPLE_BINARY_SEMAPHORE | RTEMS_PRIORITY, 0, &result_sem);
    
    if (sc != RTEMS_SUCCESSFUL) {
        printf(" 创建信号量失败\n");
        TEST_END();
        rtems_test_exit(1);
    }
    
    // 创建网络容器
    printf(" 创建网络容器...\n");
    container_main = rtems_net_container_create();
    container_sub = rtems_net_container_create();
    
    if (!container_main || !container_sub) {
        printf(" 创建网络容器失败\n");
        TEST_END();
        rtems_test_exit(1);
    }
    
    printf(" 网络容器创建成功:\n");
    printf("   - 根容器 (ID=%d)\n", container_main->containerID);
    printf("   - 子容器 (ID=%d)\n", container_sub->containerID);
    printf("\n");
    
    // 将当前任务关联到根容器
    Thread_Control *init_thread = _Thread_Get_executing();
    if (!init_thread->container) {
        init_thread->container = (Container *)_Workspace_Allocate(sizeof(Container));
        memset(init_thread->container, 0, sizeof(Container));
    }
    init_thread->container->netContainer = container_main;
    container_main->rc++;
    
    printf(" Init 任务已关联到根容器\n\n");
    
    // 根容器：绑定 UDP + TCP
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf(" 根容器绑定测试\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    int udp_sock, tcp_sock;
    main_result = bind_udp_tcp("根容器", &udp_sock, &tcp_sock);
    
    if (main_result != 0) {
        printf("\n 根容器绑定失败\n");
        rtems_net_container_delete(container_main);
        rtems_net_container_delete(container_sub);
        TEST_END();
        rtems_test_exit(1);
    }
    
    main_udp_sock = udp_sock;
    main_tcp_sock = tcp_sock;
    
    // 创建子容器任务
    sc = rtems_task_create(
        rtems_build_name('S', 'U', 'B', 'T'),
        1,
        RTEMS_MINIMUM_STACK_SIZE * 4,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &sub_task_id
    );
    
    if (sc != RTEMS_SUCCESSFUL) {
        printf(" 创建子任务失败\n");
        close(udp_sock);
        close(tcp_sock);
        rtems_net_container_delete(container_main);
        rtems_net_container_delete(container_sub);
        TEST_END();
        rtems_test_exit(1);
    }
    
    // 将子任务关联到子容器
    ISR_lock_Context lock_context;
    Thread_Control *sub_thread = _Thread_Get(sub_task_id, &lock_context);
    
    if (sub_thread) {
        if (!sub_thread->container) {
            sub_thread->container = (Container *)_Workspace_Allocate(sizeof(Container));
            memset(sub_thread->container, 0, sizeof(Container));
        }
        sub_thread->container->netContainer = container_sub;
        container_sub->rc++;
        _ISR_lock_ISR_enable(&lock_context);
    }
    
    // 启动子容器任务
    printf("\n 启动子容器任务...\n");
    sc = rtems_task_start(sub_task_id, sub_container_task, 0);
    
    if (sc != RTEMS_SUCCESSFUL) {
        printf(" 启动子任务失败\n");
        close(udp_sock);
        close(tcp_sock);
        rtems_net_container_delete(container_main);
        rtems_net_container_delete(container_sub);
        TEST_END();
        rtems_test_exit(1);
    }
    
    // 通知子容器可以开始绑定
    rtems_task_wake_after(rtems_clock_get_ticks_per_second() / 2);
    rtems_semaphore_release(sync_sem);
    
    // 等待子容器完成
    rtems_semaphore_obtain(result_sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    rtems_task_wake_after(rtems_clock_get_ticks_per_second());
    
    // 输出测试结果
    printf("\n");
    printf("                        测试结果                                 \n");

    
    printf("测试项目:\n");
    printf("  1. 根容器 UDP 绑定: %s\n", main_result == 0 ? " 成功" : " 失败");
    printf("  2. 根容器 TCP 绑定: %s\n", main_result == 0 ? " 成功" : " 失败");
    printf("  3. 子容器 UDP 绑定: %s\n", sub_result == 0 ? " 成功" : " 失败");
    printf("  4. 子容器 TCP 绑定: %s\n", sub_result == 0 ? " 成功" : " 失败");
    printf("\n");
    
    if (main_result == 0 && sub_result == 0) {
        printf(" 测试通过!\n\n");
        printf("验证项目:\n");
        printf("   根容器成功绑定 UDP + TCP 到端口 %d\n", TEST_PORT);
        printf("   子容器成功绑定 UDP + TCP 到相同端口 %d\n", TEST_PORT);
        printf("   无 EADDRINUSE (端口已被使用) 错误\n");
        printf("   容器间端口表完全隔离\n");
        printf("   网络资源隔离正常工作\n");
    } else {
        printf(" 测试失败!\n");
        printf("   可能原因: 容器端口表未正确隔离\n");
    }
    
    // 清理资源
    printf("\n 清理测试资源...\n");
    if (main_udp_sock >= 0) close(main_udp_sock);
    if (main_tcp_sock >= 0) close(main_tcp_sock);
    if (container_main) rtems_net_container_delete(container_main);
    if (container_sub) rtems_net_container_delete(container_sub);
    rtems_semaphore_delete(sync_sem);
    rtems_semaphore_delete(result_sem);
    printf(" 资源清理完成\n");
    
    printf("\n");
    printf("                        测试结束                                        \n");
    
    TEST_END();
    rtems_test_exit((main_result == 0 && sub_result == 0) ? 0 : 1);
}

/* ==================== 网络栈配置 ==================== */

struct rtems_bsdnet_config rtems_bsdnet_config = {
    NULL, NULL, 0, 0, 0, 0, 0, 0, 0,
    {"0.0.0.0"}, {"0.0.0.0"}, 0, 0, 0, 0, 0
};

struct rtems_bsdnet_config *rtems_bsdnet_config_ptr = &rtems_bsdnet_config;

/* ==================== RTEMS 配置 ==================== */

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_APPLICATION_NEEDS_LIBNETWORKING

#define CONFIGURE_MAXIMUM_TASKS 5
#define CONFIGURE_MAXIMUM_SEMAPHORES 5
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 30
#define CONFIGURE_MAXIMUM_SOCKETS 20

#define CONFIGURE_EXECUTIVE_RAM_SIZE (2 * 1024 * 1024)
#define CONFIGURE_MAXIMUM_REGIONS 10

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT

#include <rtems/confdefs.h>
