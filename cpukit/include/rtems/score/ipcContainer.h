#ifndef _RTEMS_SCORE_IPC_CONTAINER_H
#define _RTEMS_SCORE_IPC_CONTAINER_H

#include <rtems/score/objectdata.h>
#include <rtems/rtems/messagedata.h>
// #include <rtems/rtems/semdata.h>
#include <rtems/posix/mqueue.h>
#include <rtems/score/chainimpl.h>
#include <rtems/score/isrlock.h>

typedef struct IpcContainer
{
    int rc;
    int containerID;
    // 需要隔离的资源
    Objects_Information rtems_message_queue_info;
    Objects_Information posix_message_queue_info;
    Objects_Information semaphore_info;

} IpcContainer;

// typedef struct Semaphore_Control Semaphore_Control;
typedef struct _Thread_Control Thread_Control;

int rtems_ipc_container_initialize_root(IpcContainer **ipcContainer);

IpcContainer *rtems_ipc_container_create(void);

void rtems_ipc_container_delete(IpcContainer *ipcContainer);

void rtems_ipc_container_add_to_list(IpcContainer *ipcContainer);

void rtems_ipc_container_remove_from_list(IpcContainer *ipcContainer);

void rtems_ipc_container_move_task(IpcContainer *srcContainer, IpcContainer *destContainer, Thread_Control *thread);

int rtems_ipc_container_get_id(IpcContainer *ipcContainer);

int rtems_ipc_container_get_rc(IpcContainer *ipcContainer);

Objects_Information *_Message_queue_Get_information_from_container(void);
// Objects_Information *_Message_queue_Get_information_from_container(IpcContainer *ipcContainer);

Objects_Information *_POSIX_Message_queue_Get_information_from_container(void);

Objects_Information *_Semaphore_Get_information_from_container(void);

#endif