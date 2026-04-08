#ifndef UTS_CONTAINER_H
#define UTS_CONTAINER_H

// #include "atomic.h"
typedef signed int INT32;
typedef volatile INT32 Atomic;

// UTS容器只需要定义
typedef struct UtsContainer
{
    int rc;
    int ID;
    char name[50];    // 暂时设定为这个形式先,定义为主机名
} UtsContainer;

typedef struct _Thread_Control Thread_Control;

// 初始化根UTS容器
int rtems_uts_container_initialize_root(UtsContainer **utsContainer);

// 创建新的UTS容器
UtsContainer *rtems_uts_container_create(const char *name);

// 将线程从一个UTS容器中移到另一个UTS容器中
void rtems_uts_container_move_task(UtsContainer *srcContainer, UtsContainer *destContainer, Thread_Control *thread);

// 删除UTS容器
void rtems_uts_container_delete(UtsContainer *utsContainer);

// 查找线程位于哪个UTS容器中
UtsContainer *rtems_uts_container_find_by_thread(Thread_Control *thread);

// 将UTS容器添加到UTS容器列表中
void rtems_uts_container_add_to_list(UtsContainer *utsContainer);

// 从UTS容器列表中移除UTS容器
void rtems_uts_container_remove_from_list(UtsContainer *utsContainer); 

// // 获取线程所在的UTS容器信息
// ThreadContainerInfo GetThreadContainerInfo(Thread_Control *thread);

// 判断UTS容器是否存在
int rtems_uts_container_exists(UtsContainer *utsContainer);

#endif 