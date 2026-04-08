#include <rtems/io_cgroup.h>
#include <rtems/score/thread.h>
#include <stdlib.h>
#include <string.h>

static IO_Cgroup_Manager *global_io_cgroup_manager = NULL;

// 全局 cgroup ID 计数器
static uint32_t next_cgroup_id = 1;

// 创建一个的io_cgroup_manager
rtems_status_code rtems_io_cgroup_manager_create(
    uint64_t system_read_bps,
    uint64_t system_write_bps)
{
    // 先设定system_read_bps和system_write_bps为常值
    system_read_bps = system_read_bps == 0 ? 1048576 : system_read_bps;    // 默认1MB/s
    system_write_bps = system_write_bps == 0 ? 1048576 : system_write_bps; // 默认1MB/s

    if (global_io_cgroup_manager != NULL)
    {
        return RTEMS_SUCCESSFUL;
    }

    // 分配管理器内存
    IO_Cgroup_Manager *new_manager = (IO_Cgroup_Manager *)malloc(sizeof(IO_Cgroup_Manager));
    if (new_manager == NULL)
    {
        return RTEMS_NO_MEMORY;
    }

    // 初始化管理器
    memset(new_manager, 0, sizeof(IO_Cgroup_Manager));
    new_manager->io_cgroups = NULL;
    new_manager->io_cgroup_count = 0;
    new_manager->total_weight = 0;
    new_manager->system_read_bps = system_read_bps;
    new_manager->system_write_bps = system_write_bps;

    global_io_cgroup_manager = new_manager;

    return RTEMS_SUCCESSFUL;
}

// 更新 IO_Cgroup_Manager 的系统带宽
rtems_status_code rtems_io_cgroup_manager_update_bandwidth(
    uint64_t system_read_bps,
    uint64_t system_write_bps)
{
    IO_Cgroup_Manager *manager;
    IO_Cgroup_Control *current;

    // 检查管理器是否存在
    manager = global_io_cgroup_manager;
    if (manager == NULL) {
        return RTEMS_NOT_CONFIGURED;
    }

    // 参数校验
    if (system_read_bps == 0 || system_write_bps == 0) {
        return RTEMS_INVALID_NUMBER;
    }

    // 更新系统带宽
    manager->system_read_bps = system_read_bps;
    manager->system_write_bps = system_write_bps;

    // 重新计算所有 cgroup 的带宽限制
    if (manager->total_weight > 0) {
        current = manager->io_cgroups;
        while (current != NULL) {
            if (current->weight > 0) {
                // 按权重比例重新分配带宽
                current->limits.read_bps_limit =
                    (uint64_t)current->weight * manager->system_read_bps / manager->total_weight;
                current->limits.write_bps_limit =
                    (uint64_t)current->weight * manager->system_write_bps / manager->total_weight;
            }
            current = current->next;
        }
    }

    return RTEMS_SUCCESSFUL;
}

// 创建一个 IO_Cgroup
rtems_status_code rtems_io_cgroup_create(
    uint16_t io_cgroup_weight,
    const IO_Cgroup_Limit *limits,
    uint32_t *cgroup_id)
{
    rtems_status_code sc;

    // 参数检查
    if (limits == NULL || cgroup_id == NULL)
    {
        return RTEMS_INVALID_ADDRESS;
    }

    // 检查全局管理器是否已创建
    if (global_io_cgroup_manager == NULL)
    {
        return RTEMS_NOT_CONFIGURED;
    }

    // 分配内存
    IO_Cgroup_Control *new_cgroup = (IO_Cgroup_Control *)malloc(sizeof(IO_Cgroup_Control));
    if (new_cgroup == NULL)
    {
        return RTEMS_NO_MEMORY;
    }

    // 初始化 cgroup
    memset(new_cgroup, 0, sizeof(IO_Cgroup_Control));
    
    // 分配唯一 ID (简化版：不使用锁)
    new_cgroup->ID = next_cgroup_id++;
    
    new_cgroup->limits = *limits;

    // 初始化滑动窗口大小为1秒
    new_cgroup->limits.window_size_ms = 1000;

    // 初始化统计窗口开始时间
    new_cgroup->stats.current_window.window_start = rtems_clock_get_ticks_since_boot();

    // 初始化缓冲区
    new_cgroup->bdbuf_size = 64 * 1024;
    new_cgroup->bdbuf = malloc(new_cgroup->bdbuf_size);
    if (new_cgroup->bdbuf == NULL)
    {
        free(new_cgroup);
        return RTEMS_NO_MEMORY;
    }

    // 加入全局管理器
    sc = rtems_io_cgroup_add_to_manager(new_cgroup);
    if (sc != RTEMS_SUCCESSFUL)
    {
        free(new_cgroup->bdbuf);
        free(new_cgroup);
        return sc;
    }

    // 分配 cgroup 权重
    sc = rtems_io_cgroup_allocate_weight(new_cgroup, io_cgroup_weight);
    if (sc != RTEMS_SUCCESSFUL)
    {
        // 权重分配失败,回滚
        rtems_io_cgroup_remove_from_manager(new_cgroup);
        free(new_cgroup->bdbuf);
        free(new_cgroup);
        return sc;
    }

    // 返回 cgroup ID
    *cgroup_id = new_cgroup->ID;

    return RTEMS_SUCCESSFUL;
}

// 清理 IO_Cgroup 资源
void rtems_io_cgroup_finalize(IO_Cgroup_Control *io_cgroup)
{
    if (io_cgroup == NULL)
    {
        return;
    }

    // 清理线程权重链表
    Thread_Weight_Node *current = io_cgroup->thread_weights_chain;
    while (current != NULL)
    {
        Thread_Weight_Node *next = current->next;
        free(current);
        current = next;
    }

    io_cgroup->thread_weights_chain = NULL;
    io_cgroup->total_weight = 0;

    // 清理缓冲区分区
    if (io_cgroup->bdbuf != NULL)
    {
        free(io_cgroup->bdbuf);
        io_cgroup->bdbuf = NULL;
        io_cgroup->bdbuf_size = 0;
    }
}

// 删除 IO_Cgroup
void rtems_io_cgroup_delete(IO_Cgroup_Control *io_cgroup)
{
    if (io_cgroup == NULL)
    {
        return;
    }

    // 从管理器移除
    rtems_io_cgroup_remove_from_manager(io_cgroup);

    // 移除权重
    if (io_cgroup->weight > 0)
    {
        rtems_io_cgroup_remove_weight(io_cgroup);
    }

    // 清理资源
    rtems_io_cgroup_finalize(io_cgroup);

    // 释放内存
    free(io_cgroup);
}

// 分配线程的权重
rtems_status_code rtems_io_cgroup_allocate_thread_weight(
    IO_Cgroup_Control *io_cgroup,
    Thread_Control *thread,
    uint32_t weight)
{
    Thread_Weight_Node *current;
    Thread_Weight_Node *new_node;

    // 参数检查
    if (io_cgroup == NULL || thread == NULL || weight == 0)
    {
        return RTEMS_INVALID_ADDRESS;
    }

    // 检查线程是否已经存在
    current = io_cgroup->thread_weights_chain;
    while (current != NULL)
    {
        if (current->Thread_ID == thread->Object.id)
        {
            // 线程已存在，更新权重
            uint32_t old_weight = current->weight;

            // 检查新权重是否超过 cgroup 限制
            if (io_cgroup->total_weight - old_weight + weight > io_cgroup->weight)
            {
                return RTEMS_RESOURCE_IN_USE;
            }

            // 更新权重
            current->weight = weight;
            io_cgroup->total_weight = io_cgroup->total_weight - old_weight + weight;

            return RTEMS_SUCCESSFUL;
        }
        current = current->next;
    }

    // 线程不存在，检查是否可以分配新权重
    if (io_cgroup->total_weight + weight > io_cgroup->weight)
    {
        return RTEMS_RESOURCE_IN_USE;
    }

    // 分配新节点
    new_node = (Thread_Weight_Node *)malloc(sizeof(Thread_Weight_Node));
    if (new_node == NULL)
    {
        return RTEMS_NO_MEMORY;
    }

    // 初始化节点并插入双向链表头
    new_node->Thread_ID = thread->Object.id;
    new_node->weight = weight;
    new_node->next = io_cgroup->thread_weights_chain;
    new_node->prev = NULL;

    // 如果链表不为空，更新原链表头的 prev 指针
    if (io_cgroup->thread_weights_chain != NULL)
    {
        io_cgroup->thread_weights_chain->prev = new_node;
    }

    // 更新链表头指针
    io_cgroup->thread_weights_chain = new_node;
    io_cgroup->total_weight += weight;

    return RTEMS_SUCCESSFUL;
}

// 将 io_cgroup 添加到全局管理器
rtems_status_code rtems_io_cgroup_add_to_manager(
    IO_Cgroup_Control *io_cgroup)
{
    IO_Cgroup_Manager *manager;

    if (io_cgroup == NULL)
    {
        return RTEMS_INVALID_ADDRESS;
    }

    manager = global_io_cgroup_manager;
    if (manager == NULL)
    {
        return RTEMS_NOT_CONFIGURED;
    }

    // 插入到双向链表头部
    io_cgroup->next = manager->io_cgroups;
    io_cgroup->prev = NULL;

    if (manager->io_cgroups != NULL)
    {
        manager->io_cgroups->prev = io_cgroup;
    }

    manager->io_cgroups = io_cgroup;
    manager->io_cgroup_count++;

    return RTEMS_SUCCESSFUL;
}

// 从全局管理器中移除 io_cgroup
rtems_status_code rtems_io_cgroup_remove_from_manager(
    IO_Cgroup_Control *io_cgroup)
{
    IO_Cgroup_Manager *manager;

    if (io_cgroup == NULL)
    {
        return RTEMS_INVALID_ADDRESS;
    }

    manager = global_io_cgroup_manager;
    if (manager == NULL)
    {
        return RTEMS_NOT_CONFIGURED;
    }

    // 从双向链表中移除
    if (io_cgroup->prev != NULL)
    {
        io_cgroup->prev->next = io_cgroup->next;
    }
    else
    {
        // 当前节点是头节点
        manager->io_cgroups = io_cgroup->next;
    }

    if (io_cgroup->next != NULL)
    {
        io_cgroup->next->prev = io_cgroup->prev;
    }

    manager->io_cgroup_count--;

    // 清空节点的链表指针
    io_cgroup->next = NULL;
    io_cgroup->prev = NULL;

    return RTEMS_SUCCESSFUL;
}

// 分配 IO_Cgroup 的权重
rtems_status_code rtems_io_cgroup_allocate_weight(
    IO_Cgroup_Control *io_cgroup,
    uint32_t weight)
{
    IO_Cgroup_Manager *manager;
    uint32_t old_weight;

    if (io_cgroup == NULL || weight == 0)
    {
        return RTEMS_INVALID_ADDRESS;
    }

    manager = global_io_cgroup_manager;
    if (manager == NULL)
    {
        return RTEMS_NOT_CONFIGURED;
    }

    old_weight = io_cgroup->weight;

    // 更新 cgroup 权重
    io_cgroup->weight = weight;

    // 更新管理器总权重
    manager->total_weight = manager->total_weight - old_weight + weight;

    // 重新计算所有 cgroup 的带宽限制
    if (manager->total_weight > 0)
    {
        IO_Cgroup_Control *current = manager->io_cgroups;
        while (current != NULL)
        {
            if (current->weight > 0)
            {
                // 按权重比例分配带宽
                current->limits.read_bps_limit =
                    (uint64_t)current->weight * manager->system_read_bps / manager->total_weight;
                current->limits.write_bps_limit =
                    (uint64_t)current->weight * manager->system_write_bps / manager->total_weight;
            }
            current = current->next;
        }
    }

    return RTEMS_SUCCESSFUL;
}

// 释放 IO_Cgroup 的权重
rtems_status_code rtems_io_cgroup_remove_weight(
    IO_Cgroup_Control *io_cgroup)
{
    IO_Cgroup_Manager *manager;

    if (io_cgroup == NULL)
    {
        return RTEMS_INVALID_ADDRESS;
    }

    manager = global_io_cgroup_manager;
    if (manager == NULL)
    {
        return RTEMS_NOT_CONFIGURED;
    }

    // 从管理器总权重中减去
    manager->total_weight -= io_cgroup->weight;
    io_cgroup->weight = 0;

    // 重新计算其他 cgroup 的带宽
    if (manager->total_weight > 0)
    {
        IO_Cgroup_Control *current = manager->io_cgroups;
        while (current != NULL)
        {
            if (current->weight > 0)
            {
                current->limits.read_bps_limit =
                    (uint64_t)current->weight * manager->system_read_bps / manager->total_weight;
                current->limits.write_bps_limit =
                    (uint64_t)current->weight * manager->system_write_bps / manager->total_weight;
            }
            current = current->next;
        }
    }

    return RTEMS_SUCCESSFUL;
}

// 将线程添加到 io_cgroup 中
rtems_status_code rtems_io_cgroup_add_thread(
    IO_Cgroup_Control *io_cgroup,
    Thread_Control *thread,
    uint32_t weight)
{
    return rtems_io_cgroup_allocate_thread_weight(io_cgroup, thread, weight);
}

// 从 io_cgroup 中移除线程
rtems_status_code rtems_io_cgroup_remove_thread(
    IO_Cgroup_Control *io_cgroup,
    Thread_Control *thread)
{
    return rtems_io_cgroup_remove_thread_weight(io_cgroup, thread);
}

// 移除线程的权重
rtems_status_code rtems_io_cgroup_remove_thread_weight(
    IO_Cgroup_Control *io_cgroup,
    Thread_Control *thread)
{
    Thread_Weight_Node *current;

    if (io_cgroup == NULL || thread == NULL)
    {
        return RTEMS_INVALID_ADDRESS;
    }

    // 查找线程节点
    current = io_cgroup->thread_weights_chain;
    while (current != NULL)
    {
        if (current->Thread_ID == thread->Object.id)
        {
            // 从双向链表中移除
            if (current->prev != NULL)
            {
                current->prev->next = current->next;
            }
            else
            {
                // 当前是头节点
                io_cgroup->thread_weights_chain = current->next;
            }

            if (current->next != NULL)
            {
                current->next->prev = current->prev;
            }

            // 更新总权重
            io_cgroup->total_weight -= current->weight;

            // 释放节点
            free(current);

            return RTEMS_SUCCESSFUL;
        }
        current = current->next;
    }

    return RTEMS_INVALID_ID; // 线程未找到
}

// 更新滑动窗口
void rtems_io_cgroup_update_sliding_window(
    IO_Cgroup_Control *io_cgroup,
    rtems_interval current_time)
{
    rtems_interval elapsed;

    if (io_cgroup == NULL)
    {
        return;
    }

    elapsed = current_time - io_cgroup->stats.current_window.window_start;

    // 检查窗口是否过期（转换 ms 到 ticks）
    if (elapsed > rtems_clock_get_ticks_per_second() * io_cgroup->limits.window_size_ms / 1000)
    {
        // 窗口过期，重置
        io_cgroup->stats.current_window.bytes = 0;
        io_cgroup->stats.current_window.window_start = current_time;
    }
}

// 更新 IO 统计信息
void rtems_io_cgroup_update_stats(
    IO_Cgroup_Control *io_cgroup,
    IO_Cgroup_Request *request)
{
    if (io_cgroup == NULL || request == NULL)
    {
        return;
    }

    // 更新总统计
    if (request->type == IO_CGROUP_READ)
    {
        io_cgroup->stats.read.bytes += request->size;
    }
    else
    {
        io_cgroup->stats.write.bytes += request->size;
    }

    // 更新当前窗口统计
    io_cgroup->stats.current_window.bytes += request->size;
}

// 检查是否需要限速
bool rtems_io_cgroup_check_throttle(
    IO_Cgroup_Control *io_cgroup,
    IO_Cgroup_Request *request)
{
    rtems_interval current_time;
    rtems_interval elapsed;
    uint64_t current_rate;
    uint64_t limit;

    if (io_cgroup == NULL || request == NULL)
    {
        return false;
    }

    current_time = rtems_clock_get_ticks_since_boot();

    // 更新滑动窗口
    rtems_io_cgroup_update_sliding_window(io_cgroup, current_time);

    elapsed = current_time - io_cgroup->stats.current_window.window_start;
    if (elapsed == 0)
    {
        elapsed = 1; // 避免除零
    }

    // 计算当前速率
    current_rate = io_cgroup->stats.current_window.bytes / elapsed;

    // 获取限制
    if (request->type == IO_CGROUP_READ)
    {
        limit = io_cgroup->limits.read_bps_limit / rtems_clock_get_ticks_per_second();
    }
    else
    {
        limit = io_cgroup->limits.write_bps_limit / rtems_clock_get_ticks_per_second();
    }

    // 检查是否超限
    return (current_rate + request->size / elapsed) > limit;
}

// IO 请求处理
rtems_status_code rtems_io_cgroup_handle_request(
    IO_Cgroup_Control *io_cgroup,
    IO_Cgroup_Request *request)
{
    bool throttled;

    if (io_cgroup == NULL || request == NULL)
    {
        return RTEMS_INVALID_ADDRESS;
    }

    // 检查是否需要限速
    throttled = rtems_io_cgroup_check_throttle(io_cgroup, request);

    if (throttled)
    {
        // 更新限速统计
        if (request->type == IO_CGROUP_READ)
        {
            io_cgroup->stats.read.throttled++;
        }
        else
        {
            io_cgroup->stats.write.throttled++;
        }

        return RTEMS_TOO_MANY; // 需要限速
    }

    // 更新统计信息
    rtems_io_cgroup_update_stats(io_cgroup, request);

    return RTEMS_SUCCESSFUL;
}

// 设置 IO 限制
rtems_status_code rtems_io_cgroup_set_limits(
    IO_Cgroup_Control *io_cgroup,
    const IO_Cgroup_Limit *limits)
{
    if (io_cgroup == NULL || limits == NULL)
    {
        return RTEMS_INVALID_ADDRESS;
    }

    io_cgroup->limits = *limits;

    return RTEMS_SUCCESSFUL;
}

// 获取 IO 限制
rtems_status_code rtems_io_cgroup_get_limits(
    IO_Cgroup_Control *io_cgroup,
    IO_Cgroup_Limit *limits)
{
    if (io_cgroup == NULL || limits == NULL)
    {
        return RTEMS_INVALID_ADDRESS;
    }

    *limits = io_cgroup->limits;

    return RTEMS_SUCCESSFUL;
}

// 遍历所有 io_cgroup
void rtems_io_cgroup_traverse(
    void (*callback)(IO_Cgroup_Control *io_cgroup, void *data),
    void *data)
{
    IO_Cgroup_Manager *manager;
    IO_Cgroup_Control *current;

    // 获取全局管理器
    manager = global_io_cgroup_manager;
    if (manager == NULL || callback == NULL)
    {
        return;
    }

    // 遍历双向链表
    current = manager->io_cgroups;
    while (current != NULL)
    {
        // 保存 next 指针
        IO_Cgroup_Control *next = current->next;

        // 执行回调函数
        callback(current, data);

        // 移动到下一个节点
        current = next;
    }
}

// 通过 ID 获取 io_cgroup
IO_Cgroup_Control* rtems_io_cgroup_get_by_id(uint32_t ID)
{
    IO_Cgroup_Manager *manager;
    IO_Cgroup_Control *current;

    manager = global_io_cgroup_manager;
    if (manager == NULL) {
        return NULL;
    }

    // 遍历链表查找指定 ID 的 cgroup
    current = manager->io_cgroups;
    while (current != NULL) {
        if (current->ID == ID) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}
