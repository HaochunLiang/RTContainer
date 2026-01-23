#include <stdio.h>
#include <rtems.h>
#include <rtems/score/container.h>
#include <rtems/score/pidContainer.h>
#include <tmacros.h>

const char rtems_test_name[] = "PID CONTAINER DELETE & INFO TEST";

static rtems_task test_task(rtems_task_argument arg)
{
    printf("test_task %d running!\n", arg);
    rtems_task_exit();
}

static void print_thread_info(const char *msg, Thread_Control *thread)
{
    ThreadContainerInfo info = GetThreadContainerInfo(thread);
    if (info.isRoot == 1)
        printf("%s: 线程在根容器，containerID=%d, id=%d\n", msg, info.containerID, info.vid_or_id);
    else if (info.isRoot == 0)
        printf("%s: 线程在子容器，containerID=%d, vid=%d\n", msg, info.containerID, info.vid_or_id);
    else
        printf("%s: 未找到线程所在容器\n", msg);
}

static void print_all_pid_containers(void)
{
    Container *container = GetRootContainer();
    if (!container) {
        printf("GetRootContainer() == NULL\n");
        return;
    }
    printf("当前存在的PID容器ID：");
    // 根容器
    if (container->pidContainer)
        printf("%d ", container->pidContainer->containerID);

    // 其它容器
    PidContainerNode *node = container->pidContainerListHead;
    while (node) {
        if (node->pidContainer)
            printf("%d %d", node->pidContainer->containerID, node->pidContainer->rc);
        node = node->next;
    }
    printf("\n");
}

static rtems_task Init(rtems_task_argument ignored)
{
    rtems_id tid1, tid2, tid3;
    rtems_status_code sc;

    rtems_print_printer_fprintf_putc(&rtems_test_printer);
    TEST_BEGIN();

#ifdef LOSCFG_PID_CONTAINER
    Container *container = GetRootContainer();
    PidContainer *rootPidContainer = container->pidContainer;

    // 创建3个任务
    sc = rtems_task_create(rtems_build_name('T','1',' ',' '), 1, RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &tid1);
    if (sc == RTEMS_SUCCESSFUL) rtems_task_start(tid1, test_task, 1);

    sc = rtems_task_create(rtems_build_name('T','2',' ',' '), 1, RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &tid2);
    if (sc == RTEMS_SUCCESSFUL) rtems_task_start(tid2, test_task, 2);

    sc = rtems_task_create(rtems_build_name('T','3',' ',' '), 1, RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &tid3);
    if (sc == RTEMS_SUCCESSFUL) rtems_task_start(tid3, test_task, 3);

    // 查找Thread_Control指针
    Thread_Control *thread1 = NULL, *thread2 = NULL, *thread3 = NULL;
    ThreadNode *node = rootPidContainer->threadListHead;
    while (node) {
        Thread_Control *thread = (Thread_Control *)node->thread;
        if (thread->Object.id == tid1) thread1 = thread;
        if (thread->Object.id == tid2) thread2 = thread;
        if (thread->Object.id == tid3) thread3 = thread;
        node = node->next;
    }

    // 创建两个新的PID容器
    PidContainer *containerA = PidContainer_Create();
    PidContainer *containerB = PidContainer_Create();

    // 打印初始信息
    print_thread_info("初始 thread1", thread1);
    print_thread_info("初始 thread2", thread2);
    print_thread_info("初始 thread3", thread3);

    // 把 thread2、thread3 分别移到 containerA、containerB
    PidContainer_MoveTask(rootPidContainer, containerA, thread2);
    // printf("rootPidContainer rc=%d, containerA rc=%d\n", rootPidContainer->rc, containerA->rc);
    PidContainer_MoveTask(rootPidContainer, containerB, thread3);
    // printf("rootPidContainer rc=%d, containerA rc=%d\n", rootPidContainer->rc, containerA->rc);

    print_thread_info("切换后 thread1", thread1);
    print_thread_info("切换后 thread2", thread2);
    print_thread_info("切换后 thread3", thread3);

    // 再把 thread2 从 containerA 移到 containerB
    PidContainer_MoveTask(containerA, containerB, thread2);
    // printf("rootPidContainer rc=%d, containerA rc=%d\n", rootPidContainer->rc, containerA->rc);

    print_thread_info("thread2 移到 containerB 后", thread2);

    if (!PidContainer_Exists(containerA)) 
    {
        printf("containerA 已经被删除或不存在\n");
    } else {
        printf("containerA 仍然存在，ID=%d\n", containerA->containerID);
    }

    // 删除 containerB（thread2、thread3 应回到根容器）
    printf("删除 containerB（ID=%d）\n", containerB->containerID);
    DeletePidContainer(containerB);

    // 再次打印所有线程信息
    print_thread_info("删除后 thread1", thread1);
    print_thread_info("删除后 thread2", thread2);
    print_thread_info("删除后 thread3", thread3);

    // 打印当前还存在的PID容器ID
    print_all_pid_containers();
#else
    printf("LOSCFG_PID_CONTAINER not defined\n");
#endif

    TEST_END();
    rtems_test_exit(0);
}

#define CONFIGURE_APPLICATION_DOES_NOT_NEED_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_MAXIMUM_TASKS            4
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION
#define CONFIGURE_INIT
#include <rtems/confdefs.h>