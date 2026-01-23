#include <stdio.h>
#include <rtems.h>
#include <rtems/score/container.h>
#include <rtems/score/mntContainer.h>
#include <tmacros.h>
#include <unistd.h>
#include <sys/stat.h>
#include <rtems/fs.h> 
#include <rtems/libio.h> 
#include <rtems/fsmount.h> 
#include <fcntl.h>
#include <errno.h>
#include <string.h>

const char rtems_test_name[] = "MOUNT CONTAINER ISOLATION TEST";

/**
 * 辅助函数：挂载IMFS文件系统
 */
static int mount_imfs(const char *target)
{
    int result = mount(NULL, target, "imfs", RTEMS_FILESYSTEM_READ_WRITE, NULL);
    if (result != 0) {
        printf("  挂载失败 %s: errno=%d: %s\n", target, errno, strerror(errno));
    } else {
        printf("  挂载成功 %s\n", target);
    }
    return result;
}

/**
 * 辅助函数：创建文件并写入内容
 */
static void create_file(const char *path, const char *content)
{
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd >= 0) {
        write(fd, content, strlen(content));
        close(fd);
        printf("  创建文件成功: %s\n", path);
    } else {
        printf("  创建文件失败: %s (errno=%d: %s)\n", path, errno, strerror(errno));
    }
}

/**
 * 辅助函数：尝试打开文件并检查可达性
 */
static void check_file_access(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd >= 0) {
        printf("  文件可访问: %s\n", path);
        char buffer[128];
        int n = read(fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("  文件内容: %s\n", buffer);
        }
        close(fd);
    } else {
        printf("  文件不可访问: %s (errno=%d: %s)\n", path, errno, strerror(errno));
    }
}

static int setup_container_fs(const char *mount_point)
{
    // 确保目录存在
    mkdir(mount_point, 0777);
    
    // 首先尝试挂载
    printf("  尝试挂载IMFS到 %s\n", mount_point);
    int result = mount(NULL, mount_point, "imfs", RTEMS_FILESYSTEM_READ_WRITE, NULL);
    
    if (result == 0) {
        printf("  挂载成功 %s\n", mount_point);
        return 0;
    }
    
    printf("  挂载失败 %s: errno=%d: %s\n", mount_point, errno, strerror(errno));
    printf("  尝试使用chroot作为备选隔离方案...\n");
    
    // 创建必要的子目录
    char tmp_path[256];
    snprintf(tmp_path, sizeof(tmp_path), "%s/dev", mount_point);
    mkdir(tmp_path, 0777);
    
    snprintf(tmp_path, sizeof(tmp_path), "%s/tmp", mount_point);
    mkdir(tmp_path, 0777);
    
    // 执行chroot
    result = chroot(mount_point);
    if (result != 0) {
        printf("  chroot失败: %s\n", strerror(errno));
        return -1;
    }
    
    // 切换工作目录到新根
    chdir("/");
    printf("  成功使用chroot切换到%s并设置工作目录为/\n", mount_point);
    return 0;
}

static rtems_task Init(rtems_task_argument ignored)
{
    TEST_BEGIN();

#ifdef RTEMSCFG_MNT_CONTAINER
    Container *container = rtems_container_get_root();
    MntContainer *rootMnt = container->mntContainer;

    // 创建两个MNT容器
    MntContainer *mntA = rtems_mnt_container_create();
    MntContainer *mntB = rtems_mnt_container_create();

    // 获取当前线程
    Thread_Control *self = _Thread_Get_executing();
    int ret;

    // 在根文件系统中创建挂载点
    printf("在根容器中创建挂载点目录...\n");
    ret = mkdir("/mntA", 0777);
    printf("mkdir /mntA: ret=%d errno=%d\n", ret, errno);
    
    ret = mkdir("/mntB", 0777);
    printf("mkdir /mntB: ret=%d errno=%d\n", ret, errno);
    
    // 切换到容器A
    printf("\n----- 切换到容器A -----\n");
    rtems_mnt_container_move_task(rootMnt, mntA, self);
    
    // 在容器A中挂载IMFS到/mntA
    ret = setup_container_fs("/mntA");
    
    // 在容器A中创建测试文件
    create_file("/mntA/fileA.txt", "这是容器A中的文件内容\n");
    
    // 验证容器A中的文件可见性
    printf("验证容器A中文件可见性:\n");
    check_file_access("/mntA/fileA.txt");
    
    // 切换到容器B
    printf("\n----- 切换到容器B -----\n");
    rtems_mnt_container_move_task(mntA, mntB, self);
    
    // 在容器B中挂载IMFS到/mntB
    ret = setup_container_fs("/mntB");
    
    // 在容器B中创建测试文件
    create_file("/mntB/fileB.txt", "这是容器B中的文件内容\n");
    
    // 验证容器B中的文件可见性
    printf("验证容器B中文件可见性:\n");
    check_file_access("/mntB/fileB.txt");
    
    // 尝试访问容器A中的文件（应该不可访问）
    printf("尝试访问容器A中的文件:\n");
    check_file_access("/mntA/fileA.txt");
    
    // 切回容器A
    printf("\n----- 切回容器A -----\n");
    rtems_mnt_container_move_task(mntB, mntA, self);
    
    // 验证容器A中的文件可见性
    printf("验证容器A中文件可见性:\n");
    check_file_access("/mntA/fileA.txt");
    
    // 尝试访问容器B中的文件（应该不可访问）
    printf("尝试访问容器B中的文件:\n");
    check_file_access("/mntB/fileB.txt");

#else
    printf("RTEMSCFG_MNT_CONTAINER not defined\n");
#endif

    TEST_END();
    rtems_test_exit(0);
}

#define CONFIGURE_APPLICATION_DOES_NOT_NEED_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 20
#define CONFIGURE_MAXIMUM_TASKS            2
#define CONFIGURE_MAXIMUM_SEMAPHORES       2
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES   2
#define CONFIGURE_MAXIMUM_TIMERS           2
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT
#include <rtems/confdefs.h>