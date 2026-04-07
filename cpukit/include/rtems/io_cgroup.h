#ifndef _RTEMS_IO_CGROUP_H
#define _RTEMS_IO_CGROUP_H

#include <rtems.h>
#include <rtems/score/corecgroup.h>
#include <rtems/score/thread.h>
#include <rtems/diskdevs.h>
#include <rtems/bdbuf.h>

// 0 表示读操作，1 表示写操作
typedef enum
{
    IO_CGROUP_READ = 0,
    IO_CGROUP_WRITE = 1
} IO_Cgroup_Request_Type;

// 文档中写的io_req
typedef struct
{
    dev_t device;
    rtems_blkdev_bnum block;
    size_t size;
    IO_Cgroup_Request_Type type;
    rtems_interval timestamp;
} IO_Cgroup_Request;

typedef struct
{
    // 读操作统计
    struct
    {
        uint64_t bytes;
        uint64_t throttled; // 被限速的读操作次数，监控使用
    } read;
    // 写操作统计
    struct
    {
        uint64_t bytes;
        uint64_t throttled; // 被限速的写操作次数，监控使用
    } write;
    // 当前窗口统计，计算当前速率
    struct
    {
        uint64_t bytes;
        rtems_interval window_start; // 滑动窗口开始时间
    } current_window;
} IO_Cgroup_Stats;

typedef struct
{
    uint64_t read_bps_limit;  // 读操作带宽限制
    uint64_t write_bps_limit; // 写操作带宽限制
    uint32_t window_size_ms;  // 滑动窗口大小
} IO_Cgroup_Limit;

// 权重链表
typedef struct Thread_Weight_Node
{
    uint32_t weight;                 // 权重值
    uint32_t Thread_ID;              // 线程ID
    struct Thread_Weight_Node *next; // 指向下一个节点的指针
    struct Thread_Weight_Node *prev; // 指向上一个节点的指针
} Thread_Weight_Node;

typedef struct IO_Cgroup_Control
{
    uint32_t ID; // io_cgroup ID
    
    IO_Cgroup_Limit limits; // 整体的io_cgroup限制

    IO_Cgroup_Stats stats; // io_cgroup整体IO统计信息

    Thread_Weight_Node *thread_weights_chain; // 线程IO权重链表
    uint32_t total_weight;                    // 已分配总权重值
    uint32_t weight;                          // cgroup权重值

    // 缓冲区分区指针，bdbuf用于读写块设备
    void *bdbuf;
    size_t bdbuf_size;

    // 链表管理指针
    struct IO_Cgroup_Control *next;
    struct IO_Cgroup_Control *prev;
} IO_Cgroup_Control;

// IO_Cgroup全局管理器
typedef struct IO_Cgroup_Manager
{
    IO_Cgroup_Control *io_cgroups; // 链表
    uint32_t io_cgroup_count;      // io_cgroup数量
    uint32_t total_weight;         // 所有io_cgroup总权重值

    // 系统总带宽
    uint64_t system_read_bps;
    uint64_t system_write_bps;
} IO_Cgroup_Manager;

// 创建一个的io_cgroup_manager
rtems_status_code rtems_io_cgroup_manager_create(
    uint64_t system_read_bps,
    uint64_t system_write_bps);

// 更新io_cgroup_manager的系统带宽
rtems_status_code rtems_io_cgroup_manager_update_bandwidth(
    uint64_t system_read_bps,
    uint64_t system_write_bps);

// 创建一个io_cgroup
rtems_status_code rtems_io_cgroup_create(
    uint16_t io_cgroup_weight,
    const IO_Cgroup_Limit *limits,
    uint32_t *cgroup_id);

// 将io_cgroup添加到全局管理器中
rtems_status_code rtems_io_cgroup_add_to_manager(
    IO_Cgroup_Control *io_cgroup);

// 释放一个io_cgroup
void rtems_io_cgroup_delete(IO_Cgroup_Control *io_cgroup);

// 从全局管理器中移除io_cgroup
rtems_status_code rtems_io_cgroup_remove_from_manager(
    IO_Cgroup_Control *io_cgroup);

// 释放io_cgroup占用的资源
void rtems_io_cgroup_finalize(IO_Cgroup_Control *io_cgroup);

// 分配IO_Cgroup的权重
rtems_status_code rtems_io_cgroup_allocate_weight(
    IO_Cgroup_Control *io_cgroup,
    uint32_t weight);

// 释放IO_Cgroup的权重
rtems_status_code rtems_io_cgroup_remove_weight(
    IO_Cgroup_Control *io_cgroup);

// 将线程添加到io_cgroup中
rtems_status_code rtems_io_cgroup_add_thread(
    IO_Cgroup_Control *io_cgroup,
    Thread_Control *thread,
    uint32_t weight);

// 分配线程的权重
rtems_status_code rtems_io_cgroup_allocate_thread_weight(
    IO_Cgroup_Control *io_cgroup,
    Thread_Control *thread,
    uint32_t weight);

// 从io_cgroup中移除线程
rtems_status_code rtems_io_cgroup_remove_thread(
    IO_Cgroup_Control *io_cgroup,
    Thread_Control *thread);

// 移除线程的权重
rtems_status_code rtems_io_cgroup_remove_thread_weight(
    IO_Cgroup_Control *io_cgroup,
    Thread_Control *thread);

// IO请求的处理
rtems_status_code rtems_io_cgroup_handle_request(
    IO_Cgroup_Control *io_cgroup,
    IO_Cgroup_Request *request);

// 检查是否超限
bool rtems_io_cgroup_check_throttle(
    IO_Cgroup_Control *io_cgroup,
    IO_Cgroup_Request *request);

// 更新IO统计信息
void rtems_io_cgroup_update_stats(
    IO_Cgroup_Control *io_cgroup,
    IO_Cgroup_Request *request);

// 设置IO带宽限制与滑动窗口大小
rtems_status_code rtems_io_cgroup_set_limits(
    IO_Cgroup_Control *io_cgroup,
    const IO_Cgroup_Limit *limits);

// 获取当前IO带宽限制与滑动窗口大小
rtems_status_code rtems_io_cgroup_get_limits(
    IO_Cgroup_Control *io_cgroup,
    IO_Cgroup_Limit *limits);

// 检查是否超过带宽限制
bool rtems_io_cgroup_check_throttle(
    IO_Cgroup_Control *io_cgroup,
    IO_Cgroup_Request *request);

// 滑动窗口更新
void rtems_io_cgroup_update_sliding_window(
    IO_Cgroup_Control *io_cgroup,
    rtems_interval current_time);

// 遍历所有io_cgroup
void rtems_io_cgroup_traverse(
    void (*callback)(IO_Cgroup_Control *io_cgroup, void *data),
    void *data);

// 通过ID获取io_cgroup
IO_Cgroup_Control* rtems_io_cgroup_get_by_id(uint32_t id);

#endif /* _RTEMS_IO_CGROUP_H */
