#ifndef MNT_CONTAINER_H
#define MNT_CONTAINER_H

#include <rtems/fs.h>
#include <rtems/chain.h>

typedef struct MntContainer {
    int rc;
    int ID;
    rtems_chain_control mountList;
} MntContainer;

typedef struct _Thread_Control Thread_Control;

// ---------相关函数---------
// 初始化根mount容器
int rtems_mnt_container_initialize_root(MntContainer **mntContainer);

// 创建子mount容器
MntContainer *rtems_mnt_container_create(void);

// 创建子mount容器并继承父容器的挂载点
MntContainer *rtems_mnt_container_create_with_inheritance(MntContainer *parent);

// 删除子mount容器
void rtems_mnt_container_delete(MntContainer *mntContainer);

// 将mnt容器添加到链表中
void rtems_mnt_container_add_to_list(MntContainer *mntContainer);

// 从链表中移除mnt容器
void rtems_mnt_container_remove_from_list(MntContainer *mntContainer);

// 将当前线程的mount容器设置为指定的容器
void rtems_mnt_container_move_task(MntContainer *srcContainer, MntContainer *destContainer, Thread_Control *thread);

// 获取容器ID
int rtems_mnt_container_get_id(MntContainer *container);

// 获取容器引用计数
int rtems_mnt_container_get_rc(MntContainer *mntContainer);

MntContainer *get_current_thread_mnt_container(void);

rtems_filesystem_global_location_t *get_container_root_location(void);

rtems_filesystem_global_location_t *get_container_current_location(void);

// 路径解析辅助函数
void set_current_eval_path(const char *path);
void clear_current_eval_path(void);
const char *get_adjusted_eval_path(void);

#endif