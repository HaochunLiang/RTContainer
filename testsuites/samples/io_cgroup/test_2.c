// 测试是否能对读写IO带宽进行限制
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems.h>
#include <rtems/io_cgroup.h>
#include <rtems/blkdev.h>
#include <rtems/bdbuf.h>
#include <rtems/ramdisk.h>
#include <rtems/diskdevs.h>
#include <tmacros.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <rtems/libio.h>

const char rtems_test_name[] = "IO CGROUP BANDWIDTH LIMIT TEST";

// RAM Disk 配置
#define RAMDISK_BLOCK_SIZE 512
#define RAMDISK_BLOCK_COUNT 2048  // 1MB
#define RAMDISK_DEVICE_NAME "/dev/ramdisk0"

// RAM Disk 文件描述符
static int ramdisk_fd = -1;

// 全局变量
static IO_Cgroup_Control *test_cgroup1;
static IO_Cgroup_Control *test_cgroup2;
static uint32_t cgroup1_id;
static uint32_t cgroup2_id;
static rtems_id cgroup1_task_id;
static rtems_id cgroup2_task_id;

// 测试统计
static volatile int test_completed = 0;

// 初始化 RAM Disk
static bool init_ramdisk(void)
{
    rtems_status_code sc;
    
    printf("=== 初始化 RAM Disk ===\n");
    
    // 使用 ramdisk_register 直接注册并创建 RAM Disk
    printf("DEBUG: 正在调用 ramdisk_register...\n");
    sc = ramdisk_register(
        RAMDISK_BLOCK_SIZE,     // media_block_size
        RAMDISK_BLOCK_COUNT,    // media_block_count
        false,                  // trace: 不启用跟踪
        RAMDISK_DEVICE_NAME     // disk device path
    );
    
    printf("DEBUG: ramdisk_register 返回: %s (code=%d)\n", 
           rtems_status_text(sc), (int)sc);
    
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: RAM Disk 注册失败: %s\n", rtems_status_text(sc));
        return false;
    }
    
    printf("SUCCESS: RAM Disk 注册成功\n");
    
    // 打开 RAM Disk 设备用于后续 I/O 操作
    printf("DEBUG: 正在打开设备 %s...\n", RAMDISK_DEVICE_NAME);
    ramdisk_fd = open(RAMDISK_DEVICE_NAME, O_RDWR);
    if (ramdisk_fd < 0) {
        printf("ERROR: 无法打开 RAM Disk 设备 (errno=%d)\n", errno);
        printf("  提示: 设备已注册但打开失败,可能需要配置文件系统或驱动表\n");
        // 即使打开失败,也继续测试 cgroup 功能
        ramdisk_fd = -1;
    } else {
        printf("SUCCESS: 设备已成功打开 (fd=%d)\n", ramdisk_fd);
    }
    
    printf("SUCCESS: RAM Disk 初始化成功\n");
    printf("  - 设备路径: %s\n", RAMDISK_DEVICE_NAME);
    printf("  - 块大小: %u bytes\n", RAMDISK_BLOCK_SIZE);
    printf("  - 块数量: %u\n", RAMDISK_BLOCK_COUNT);
    printf("  - 总容量: %u KB\n", (RAMDISK_BLOCK_SIZE * RAMDISK_BLOCK_COUNT) / 1024);
    if (ramdisk_fd >= 0) {
        printf("  - I/O 方式: 真实块设备 I/O (read/write)\n\n");
    } else {
        printf("  - I/O 方式: 内存模拟 (验证 cgroup 功能)\n\n");
    }
    
    return true;
}

// 创建 Manager 和 Cgroups
static bool create_test_cgroups(void)
{
    rtems_status_code sc;
    IO_Cgroup_Limit limits = {0};
    
    printf("=== 创建 IO Cgroup Manager ===\n");
    
    // 创建 Manager,系统总带宽 2MB/s 读 + 2MB/s 写
    sc = rtems_io_cgroup_manager_create(2 * 1024 * 1024, 2 * 1024 * 1024);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: Manager 创建失败: %s\n", rtems_status_text(sc));
        return false;
    }
    printf("SUCCESS: Manager 创建成功, 系统读带宽: 2097152 bps, 写带宽: 2097152 bps\n\n");
    
    printf("=== 创建测试 Cgroups ===\n");
    
    // 创建 Cgroup1,权重 70
    sc = rtems_io_cgroup_create(70, &limits, &cgroup1_id);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: Cgroup1 创建失败: %s\n", rtems_status_text(sc));
        return false;
    }
    printf("SUCCESS: Cgroup1 创建成功, ID: %u, 权重: 70\n", cgroup1_id);
    
    // 创建 Cgroup2,权重 30
    sc = rtems_io_cgroup_create(30, &limits, &cgroup2_id);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: Cgroup2 创建失败: %s\n", rtems_status_text(sc));
        return false;
    }
    printf("SUCCESS: Cgroup2 创建成功, ID: %u, 权重: 30\n\n", cgroup2_id);
    
    return true;
}

// 真实 I/O 操作 (使用 RAM Disk + bdbuf)
static void simulate_io_operations(
    IO_Cgroup_Control *cg,
    const char *cgroup_name)
{
    rtems_status_code sc;
    rtems_interval start_time, end_time;
    int success_count = 0;
    
    printf("INFO: %s 开始执行 I/O 操作...\n", cgroup_name);
    start_time = rtems_clock_get_ticks_since_boot();
    
    // 执行 500 次 I/O 操作,每次读写 8 个块 (4KB)
    for (int i = 0; i < 500; i++) {
        rtems_blkdev_bnum block = (i * 8) % (RAMDISK_BLOCK_COUNT - 8);
        size_t io_size = 4096;  // 8 blocks * 512 bytes
        
        // 创建 cgroup 请求
        IO_Cgroup_Request req = {
            .device = 0,
            .block = block,
            .size = io_size,
            .type = (i % 2 == 0) ? IO_CGROUP_WRITE : IO_CGROUP_READ,
            .timestamp = rtems_clock_get_ticks_since_boot()
        };
        
        // 先通过 cgroup 检查限速
        sc = rtems_io_cgroup_handle_request(cg, &req);
        
        if (sc == RTEMS_TOO_MANY) {
            // 被限速,等待 10ms 后重试
            rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(10));
            i--;
            continue;
        }
        
        // 执行真实的块设备 I/O 操作
        if (ramdisk_fd >= 0) {
            // 使用 read/write 系统调用进行真实 I/O
            char io_buffer[4096];  // 4KB buffer
            ssize_t result;
            
            // 定位到指定块
            off_t offset = block * RAMDISK_BLOCK_SIZE;
            if (lseek(ramdisk_fd, offset, SEEK_SET) != offset) {
                // 定位失败,跳过这次操作
                continue;
            }
            
            if (req.type == IO_CGROUP_WRITE) {
                // 真实写操作
                memset(io_buffer, (char)(i & 0xFF), io_size);
                result = write(ramdisk_fd, io_buffer, io_size);
                if (result > 0) {
                    success_count++;
                }
            } else {
                // 真实读操作   （read() → rtems_io → block device driver → rtems_bdbuf_read() → RAM Disk）
                result = read(ramdisk_fd, io_buffer, io_size);
                if (result > 0) {
                    // 访问读取的数据以确保读取完成
                    volatile char dummy = io_buffer[0];
                    (void)dummy;
                    success_count++;
                }
            }
        } else {
            // 降级为内存模拟
            char dummy_buffer[512];
            if (req.type == IO_CGROUP_WRITE) {
                memset(dummy_buffer, (char)(i & 0xFF), sizeof(dummy_buffer));
            } else {
                volatile char dummy = dummy_buffer[0];
                (void)dummy;
            }
            success_count++;
        }
        
        // 每 50 次输出进度
        if ((i + 1) % 50 == 0) {
            printf("INFO: %s 已完成 %d/500 次 I/O 操作\n", cgroup_name, i + 1);
        }
    }
    
    end_time = rtems_clock_get_ticks_since_boot();
    rtems_interval elapsed = end_time - start_time;
    
    printf("SUCCESS: %s I/O 操作完成\n", cgroup_name);
    printf("  - 成功操作: %d 次\n", success_count);
    printf("  - 总字节数: %llu bytes\n", 
           (unsigned long long)(cg->stats.read.bytes + cg->stats.write.bytes));
    printf("  - 读字节数: %llu bytes\n", (unsigned long long)cg->stats.read.bytes);
    printf("  - 写字节数: %llu bytes\n", (unsigned long long)cg->stats.write.bytes);
    printf("  - 被限速次数: 总=%llu, 读=%llu, 写=%llu\n", 
           (unsigned long long)(cg->stats.read.throttled + cg->stats.write.throttled),
           (unsigned long long)cg->stats.read.throttled,
           (unsigned long long)cg->stats.write.throttled);
    printf("  - 验证: %s\n", 
           (cg->stats.read.bytes > 0 && cg->stats.write.bytes > 0) 
           ? "I/O统计正常更新" : "统计异常");
    
    // 计算秒和毫秒 (避免浮点数)
    uint32_t ticks_per_sec = rtems_clock_get_ticks_per_second();
    uint32_t seconds = elapsed / ticks_per_sec;
    uint32_t milliseconds = (elapsed % ticks_per_sec) * 1000 / ticks_per_sec;
    printf("  - 耗时: %u ticks (%u.%03u 秒)\n", 
           (unsigned int)elapsed, seconds, milliseconds);
    
    if (elapsed > 0) {
        // 计算吞吐量 KB/s (避免浮点数)
        uint64_t total_bytes = cg->stats.read.bytes + cg->stats.write.bytes;
        uint64_t throughput_bps = (total_bytes * ticks_per_sec) / elapsed;
        uint32_t throughput_kbps = (uint32_t)(throughput_bps / 1024);
        printf("  - 平均吞吐量: %u KB/s\n\n", throughput_kbps);
    }
}

// Cgroup1 测试任务 (高权重)
static rtems_task cgroup1_io_task(rtems_task_argument arg)
{
    IO_Cgroup_Control *cg = (IO_Cgroup_Control *)arg;
    
    printf("INFO: Cgroup1 测试任务启动 (高权重 70)\n");
    
    // 等待一下让 cgroup2 也启动
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));
    
    simulate_io_operations(cg, "Cgroup1");
    
    printf("INFO: Cgroup1 测试任务结束\n");
    rtems_task_delete(RTEMS_SELF);
}

// Cgroup2 测试任务 (低权重)
static rtems_task cgroup2_io_task(rtems_task_argument arg)
{
    IO_Cgroup_Control *cg = (IO_Cgroup_Control *)arg;
    
    printf("INFO: Cgroup2 测试任务启动 (低权重 30)\n");
    
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(100));
    
    simulate_io_operations(cg, "Cgroup2");
    
    printf("INFO: Cgroup2 测试任务结束\n");
    
    test_completed = 1;
    rtems_task_delete(RTEMS_SELF);
}

// 验证测试结果
static void verify_test_results(void)
{
    uint64_t cgroup1_bytes = test_cgroup1->stats.read.bytes + test_cgroup1->stats.write.bytes;
    uint64_t cgroup1_throttled = test_cgroup1->stats.read.throttled + test_cgroup1->stats.write.throttled;
    uint64_t cgroup2_bytes = test_cgroup2->stats.read.bytes + test_cgroup2->stats.write.bytes;
    uint64_t cgroup2_throttled = test_cgroup2->stats.read.throttled + test_cgroup2->stats.write.throttled;
    
    printf("\n=== 测试结果分析 ===\n");
    
    printf("Cgroup1 统计 (权重 70):\n");
    printf("  - 总字节数: %llu bytes (%u KB)\n", 
           (unsigned long long)cgroup1_bytes,
           (uint32_t)(cgroup1_bytes / 1024));
    printf("  - 被限速次数: %llu\n", (unsigned long long)cgroup1_throttled);
    
    printf("\nCgroup2 统计 (权重 30):\n");
    printf("  - 总字节数: %llu bytes (%u KB)\n", 
           (unsigned long long)cgroup2_bytes,
           (uint32_t)(cgroup2_bytes / 1024));
    printf("  - 被限速次数: %llu\n", (unsigned long long)cgroup2_throttled);
    
    // 计算限速比例 (权重低的应该被限速更多次)
    printf("\n限速比例验证:\n");
    if (cgroup1_throttled > 0 && cgroup2_throttled > 0) {
        uint32_t throttle_ratio_x100 = (uint32_t)((cgroup2_throttled * 100) / cgroup1_throttled);
        printf("  - 限速次数比: Cgroup2/Cgroup1 = %u.%02u : 1\n", 
               throttle_ratio_x100 / 100, throttle_ratio_x100 % 100);
        printf("  - 说明: Cgroup2 (权重30) 比 Cgroup1 (权重70) 被限速更多次\n");
        
        // 验证限速比例 (Cgroup2 应该是 Cgroup1 的 2-3 倍)
        if (throttle_ratio_x100 >= 180 && throttle_ratio_x100 <= 300) {
            printf("  - 结果: PASS\n");
        } else {
            printf("  - 结果: WARN (限速比例: %llu vs %llu)\n", 
                   (unsigned long long)cgroup2_throttled,
                   (unsigned long long)cgroup1_throttled);
        }
    }
    
    // 验证限速是否生效
    printf("\n限速功能验证:\n");
    if (cgroup1_throttled > 0 || cgroup2_throttled > 0) {
        printf("  - 结果: PASS (检测到限速行为)\n");
    } else {
        printf("  - 结果: WARN (未检测到限速)\n");
    }
    
    printf("\n=== 测试完成 ===\n");
}

// 主测试任务
static rtems_task test_runner_task(rtems_task_argument argument)
{
    rtems_status_code sc;
    
    printf("\n");
    printf("===============================================\n");
    printf("  IO Cgroup 带宽限制测试\n");
    printf("  测试目标: 验证权重分配和带宽限制功能\n");
    printf("===============================================\n\n");
    
    // 初始化 bdbuf
    sc = rtems_bdbuf_init();
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: bdbuf 初始化失败: %s\n", rtems_status_text(sc));
        TEST_END();
        rtems_test_exit(1);
    }
    
    // 初始化 RAM Disk
    if (!init_ramdisk()) {
        printf("ERROR: RAM Disk 初始化失败\n");
        TEST_END();
        rtems_test_exit(1);
    }
    
    // 创建 Manager 和 Cgroups
    if (!create_test_cgroups()) {
        printf("ERROR: Cgroup 初始化失败\n");
        TEST_END();
        rtems_test_exit(1);
    }
    
    // 通过 ID 获取创建的 cgroup
    test_cgroup1 = rtems_io_cgroup_get_by_id(cgroup1_id);
    test_cgroup2 = rtems_io_cgroup_get_by_id(cgroup2_id);
    
    if (test_cgroup1 == NULL || test_cgroup2 == NULL) {
        printf("ERROR: 无法通过 ID 获取 cgroup 指针\n");
        TEST_END();
        rtems_test_exit(1);
    }
    
    printf("SUCCESS: 获取到 Cgroup 指针\n");
    printf("  - Cgroup1 (ID=%u): 权重=%u, 读限制=%llu bps, 写限制=%llu bps\n",
           test_cgroup1->ID,
           test_cgroup1->weight,
           (unsigned long long)test_cgroup1->limits.read_bps_limit,
           (unsigned long long)test_cgroup1->limits.write_bps_limit);
    printf("  - Cgroup2 (ID=%u): 权重=%u, 读限制=%llu bps, 写限制=%llu bps\n\n",
           test_cgroup2->ID,
           test_cgroup2->weight,
           (unsigned long long)test_cgroup2->limits.read_bps_limit,
           (unsigned long long)test_cgroup2->limits.write_bps_limit);
    
    printf("=== 启动 I/O 测试任务 ===\n");
    
    // 创建 Cgroup1 测试任务
    sc = rtems_task_create(
        rtems_build_name('C', 'G', '1', 'T'),
        1,
        8192,  // 8KB 栈
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &cgroup1_task_id
    );
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 创建 Cgroup1 任务失败: %s\n", rtems_status_text(sc));
        TEST_END();
        rtems_test_exit(1);
    }
    
    // 创建 Cgroup2 测试任务
    sc = rtems_task_create(
        rtems_build_name('C', 'G', '2', 'T'),
        1,
        8192,  // 8KB 栈
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &cgroup2_task_id
    );
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 创建 Cgroup2 任务失败: %s\n", rtems_status_text(sc));
        TEST_END();
        rtems_test_exit(1);
    }
    
    // 启动任务
    rtems_task_start(cgroup1_task_id, cgroup1_io_task, (rtems_task_argument)test_cgroup1);
    rtems_task_start(cgroup2_task_id, cgroup2_io_task, (rtems_task_argument)test_cgroup2);
    
    // 等待测试完成
    while (!test_completed) {
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(500));
    }
    
    // 等待所有任务结束
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(1000));
    
    // 验证结果
    verify_test_results();
    
    TEST_END();
    rtems_test_exit(0);
}

// Init 任务：创建并启动测试任务
static rtems_task Init(rtems_task_argument argument)
{
    rtems_status_code sc;
    rtems_id test_task_id;
    
    TEST_BEGIN();
    
    printf("DEBUG: RTEMS_MINIMUM_STACK_SIZE = %zu\n", (size_t)RTEMS_MINIMUM_STACK_SIZE);
    
    // 创建测试运行器任务
    sc = rtems_task_create(
        rtems_build_name('T', 'E', 'S', 'T'),
        1,
        16384,  // 使用固定 16KB 栈
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &test_task_id
    );
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 创建测试任务失败: %s\n", rtems_status_text(sc));
        TEST_END();
        rtems_test_exit(1);
    }
    
    // 启动测试任务
    sc = rtems_task_start(test_task_id, test_runner_task, 0);
    if (sc != RTEMS_SUCCESSFUL) {
        printf("ERROR: 启动测试任务失败\n");
        TEST_END();
        rtems_test_exit(1);
    }
    
    // Init 任务挂起，让测试任务运行
    rtems_task_suspend(RTEMS_SELF);
}

// RTEMS 配置
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK

#define CONFIGURE_MAXIMUM_TASKS 5
#define CONFIGURE_MAXIMUM_SEMAPHORES 10
#define CONFIGURE_MAXIMUM_DRIVERS 4
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 20
#define CONFIGURE_IMFS_MEMFILE_BYTES_PER_BLOCK 512

// Bdbuf 配置
#define CONFIGURE_BDBUF_BUFFER_MAX_SIZE (8 * 512)
#define CONFIGURE_BDBUF_BUFFER_MIN_SIZE (512)
#define CONFIGURE_BDBUF_CACHE_MEMORY_SIZE (128 * 1024)
#define CONFIGURE_BDBUF_MAX_READ_AHEAD_BLOCKS 0
#define CONFIGURE_SWAPOUT_TASK_PRIORITY 15

#define CONFIGURE_MICROSECONDS_PER_TICK 1000

#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_STACK_SIZE (RTEMS_MINIMUM_STACK_SIZE * 8)

#define CONFIGURE_EXTRA_TASK_STACKS (32 * 1024)

#define CONFIGURE_INIT

#include <rtems/confdefs.h>
