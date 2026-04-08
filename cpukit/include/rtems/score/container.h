#ifndef _RTEMS_SCORE_CONTAINER_H
#define _RTEMS_SCORE_CONTAINER_H

#include <rtems/sysinit.h>
#include <stddef.h>
// #include <stdlib.h>
#ifdef RTEMSCFG_PID_CONTAINER
#include "pidContainer.h"
#endif
#ifdef RTEMSCFG_UTS_CONTAINER
#include "utsContainer.h"
#endif
#ifdef RTEMSCFG_MNT_CONTAINER
#include "mntContainer.h"
#endif
#ifdef RTEMSCFG_NET_CONTAINER
#include "netContainer.h"
#endif
#ifdef RTEMSCFG_IPC_CONTAINER
#include "ipcContainer.h"
#endif

typedef signed int INT32;
typedef volatile INT32 Atomic;

#ifdef RTEMSCFG_PID_CONTAINER
typedef struct PidContainerNode
{
    struct PidContainer *pidContainer;
    struct PidContainerNode *next;
} PidContainerNode;
#endif

#ifdef RTEMSCFG_UTS_CONTAINER
typedef struct UtsContainerNode
{
    UtsContainer *utsContainer;
    struct UtsContainerNode *next;
} UtsContainerNode;
#endif

#ifdef RTEMSCFG_MNT_CONTAINER
typedef struct MntContainerNode
{
    MntContainer *mntContainer;
    struct MntContainerNode *next;
} MntContainerNode;
#endif

// typedef struct IpcContainerNode
// {
//     IpcContainer *ipcContainer;
//     struct IpcContainerNode *next;
// } IpcContainerNode;

#ifdef RTEMSCFG_NET_CONTAINER
typedef struct NetContainerNode {
    NetContainer *netContainer;
    struct NetContainerNode *next;
} NetContainerNode;
#endif

#ifdef RTEMSCFG_IPC_CONTAINER
typedef struct IpcContainerNode {
    IpcContainer *ipcContainer;
    struct IpcContainerNode *next;
} IpcContainerNode;
#endif

typedef struct Container
{
    Atomic rc;
#ifdef RTEMSCFG_PID_CONTAINER
    struct PidContainer *pidContainer;      // 根pid容器
    PidContainerNode *pidContainerListHead; // 非根pid容器链表头
    // struct PidContainer *pidForChildContainer;
#endif
#ifdef RTEMSCFG_UTS_CONTAINER
    struct UtsContainer *utsContainer;      // 容器对应的uts容器
    UtsContainerNode *utsContainerListHead; // 非根uts容器链表头，只有根uts容器存有
#endif
#ifdef RTEMSCFG_MNT_CONTAINER
    struct MntContainer *mntContainer;      // 容器对应的mnt容器
    MntContainerNode *mntContainerListHead; // 非根mnt容器链表头，只有根mnt容器存有
#endif
#ifdef RTEMSCFG_NET_CONTAINER
    struct NetContainer *netContainer;      // 容器对应的net容器
    NetContainerNode *netContainerListHead; // 非根net容器链表头，只有根net容器存有
#endif
#ifdef RTEMSCFG_IPC_CONTAINER
    struct IpcContainer *ipcContainer;      // 容器对应的ipc容器
    IpcContainerNode *ipcContainerListHead; // 非根ipc容器链表头，只有根ipc容器存有
#endif
} Container;

void rtems_container_initialize_root(void);
Container *rtems_container_get_root(void);

#endif /* _RTEMS_SCORE_CONTAINER_H */