#ifndef PID_CONTAINER_H
#define PID_CONTAINER_H

#include "atomic.h"
#include <rtems/score/object.h>
// #include <rtems/score/threadimpl.h>
// #include <rtems/score/thread.h>
//#include "config.h"   
// #include "rtems/score/threadq.h"
// #include "rtems/score/thread.h"
// #include <rtems/score/threadimpl.h> 
// struct Thread_Control;

typedef signed int INT32;
typedef volatile INT32 Atomic;
// typedef signed int objects_Id;

#define RTEMS_MAX_PROCESS_LIMIT 1024

// #ifdef RTEMSCFG_PID_CONTAINER

typedef struct {
    INT32 containerID;
    INT32 vid_or_id;
    int isRoot;         // 1=根容器，0=子容器，-1=未找到
} ThreadContainerInfo;

typedef struct PidContainer PidContainer;   
typedef struct _Thread_Control Thread_Control;

// 初始化根PID容器
int rtems_pid_container_initialize_root(PidContainer **pidContainer);

// 创建新的PID容器
PidContainer *rtems_pid_container_create(void);

// 添加线程到容器中
void rtems_pid_container_add_task(PidContainer *container, Thread_Control *thread);   

// 从容器中移除线程
void rtems_pid_container_remove_task(PidContainer *container, Thread_Control *thread);

// 将线程从一个PID容器中移到另一个PID容器中
void rtems_pid_container_move_task(PidContainer *srcContainer, PidContainer *destContainer, Thread_Control *thread);

// 将PID容器添加到PID容器列表中
void rtems_pid_container_add_to_list(PidContainer *pidContainer);

// 从PID容器列表中移除PID容器
void rtems_pid_container_remove_from_list(PidContainer *pidContainer); 

// 删除PID容器
void rtems_pid_container_delete(PidContainer *pidContainer);

// 查找线程位于哪个PID容器中
PidContainer *rtems_pid_container_find_by_thread(Thread_Control *thread);

// 获取线程所在的PID容器信息
ThreadContainerInfo rtems_pid_container_get_thread_info(Thread_Control *thread);

// 判断PID容器是否存在
int rtems_pid_container_exists(PidContainer *pidContainer);

// #endif
void rtems_pid_container_inc_rc(PidContainer *container);

int rtems_pid_container_get_id(PidContainer *container);

int rtems_pid_container_get_rc(PidContainer *container);

void rtems_pid_container_foreach_thread(PidContainer *container, bool (*visitor)(Thread_Control *, void *), void *arg);

#endif 