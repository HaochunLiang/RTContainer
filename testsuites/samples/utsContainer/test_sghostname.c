#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <rtems.h>
#include <rtems/score/container.h>
#include <rtems/score/utsContainer.h>
#include <rtems/score/threadimpl.h>
#include <tmacros.h>

const char rtems_test_name[] = "SGHOSTNAME TEST";

static rtems_task Init(rtems_task_argument ignored)
{
    char hostname[256];
    Thread_Control *self;

    rtems_print_printer_fprintf_putc(&rtems_test_printer);
    TEST_BEGIN();

#ifdef RTEMSCFG_UTS_CONTAINER
    Container *container = rtems_container_get_root();
    printf("根容器地址: %p\n", (void*)container);
    
    if (container && container->utsContainer) {
        printf("根容器UTS名称: %s\n", container->utsContainer->name);
    }

    // 测试1: 在根容器中测试 gethostname
    printf("\n=== 测试1: 根容器中的gethostname ===\n");
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("根容器主机名: %s\n", hostname);
    } else {
        printf("根容器中gethostname失败\n");
    }

    // 测试2: 在根容器中测试 sethostname
    printf("\n=== 测试2: 根容器中的sethostname ===\n");
    if (sethostname("root-host", 9) == 0) {
        printf("sethostname成功\n");
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            printf("根容器新主机名: %s\n", hostname);
        }
    } else {
        printf("sethostname失败\n");
    }

    // 测试3: 创建新的UTS容器
    printf("\n=== 测试3: 创建新的UTS容器 ===\n");
    UtsContainer *newUts = rtems_uts_container_create("container1");
    printf("已创建UTS容器: %p, 名称: %s\n", (void*)newUts, newUts->name);

    // 测试4: 切换到新容器并测试 gethostname
    printf("\n=== 测试4: 切换到新容器并测试gethostname ===\n");
    self = (Thread_Control *)_Thread_Get_executing();
    printf("切换前，通过gethostname获取主机名: ");
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("%s\n", hostname);
    }
    
    rtems_uts_container_move_task(self->container->utsContainer, newUts, self);
    printf("已切换到容器: %s\n", self->container->utsContainer->name);
    
    printf("切换后，通过gethostname获取主机名: ");
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("%s\n", hostname);
    }

    // 测试5: 在新容器中使用 sethostname
    printf("\n=== 测试5: 新容器中的sethostname ===\n");
    if (sethostname("myhost123", 9) == 0) {
        printf("新容器中sethostname成功\n");
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            printf("容器中的新主机名: %s\n", hostname);
        }
    } else {
        printf("新容器中sethostname失败\n");
    }

    // 测试6: 多容器切换验证（通过手动管理rc避免提前删除）
    printf("\n=== 测试6: 手动管理rc的多容器切换 ===\n");
    
    // 手动增加container1的rc，防止在切换时被自动删除
    newUts->rc++;
    printf("手动增加container1的rc至: %d\n", newUts->rc);
    
    // 当前在container1，切换到root
    rtems_uts_container_move_task(self->container->utsContainer, container->utsContainer, self);
    printf("已切换到根容器 (container1未删除, rc=%d)\n", newUts->rc);
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("根容器主机名: %s\n", hostname);
    }
    
    // 切换回container1
    rtems_uts_container_move_task(self->container->utsContainer, newUts, self);
    printf("已切换回container1\n");
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("Container1主机名: %s\n", hostname);
    }

    // 测试7: 创建第二个容器并测试隔离性
    printf("\n=== 测试7: 创建container2并测试隔离性 ===\n");
    
    UtsContainer *uts2 = rtems_uts_container_create("container2");
    printf("已创建container2 (ID: %d)\n", uts2->ID);
    
    // 切换到container2并设置不同的hostname
    rtems_uts_container_move_task(self->container->utsContainer, uts2, self);
    printf("已切换到container2\n");
    if (sethostname("isolated-host", 13) == 0) {
        printf("在container2中设置主机名: isolated-host\n");
    }
    
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("Container2主机名: %s\n", hostname);
    }
    
    // 切换回container1验证隔离性
    rtems_uts_container_move_task(self->container->utsContainer, newUts, self);
    printf("已切换回container1\n");
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("Container1主机名 (应为myhost123): %s\n", hostname);
    }

    // 测试8: 手动减少rc并清理container1
    printf("\n=== 测试8: 手动清理container1 ===\n");
    newUts->rc--;  // 减少之前手动增加的rc
    printf("手动减少container1的rc至: %d\n", newUts->rc);
    
    // 切换到root，此时container1的rc会变为0，触发自动删除
    rtems_uts_container_move_task(self->container->utsContainer, container->utsContainer, self);
    printf("已切换到根容器 (container1自动删除, rc=0)\n");
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("根容器主机名: %s\n", hostname);
    }

#else
    printf("未定义RTEMSCFG_UTS_CONTAINER\n");
    printf("此测试需要UTS容器支持\n");
#endif

    TEST_END();
    rtems_test_exit(0);
}

#define CONFIGURE_APPLICATION_DOES_NOT_NEED_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_MAXIMUM_TASKS            5
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
