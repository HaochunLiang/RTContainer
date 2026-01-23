#include <rtems/score/container.h>
#include <rtems/score/smp.h>
#include <rtems/score/percpu.h>
#include <rtems/score/thread.h>
#include <stdlib.h>

static Container rootContainerObj;      // 用来避免野指针和内存泄漏
static Container *rootContainer = NULL;

void rtems_container_initialize_root(void){

    if (rootContainer != NULL) {
        return; 
    }
    // rootContainer = (Container *)malloc(sizeof(Container));
    rootContainer = &rootContainerObj;

    rootContainer->rc = 1; 
#ifdef RTEMSCFG_PID_CONTAINER
    rootContainer->pidContainer = NULL; // 初始化pidContainer为NULL 
    rootContainer->pidContainerListHead = NULL;
    (void)rtems_pid_container_initialize_root(&rootContainer->pidContainer);
#endif

#ifdef RTEMSCFG_UTS_CONTAINER
    rootContainer->utsContainer = NULL; // 初始化utsContainer为NULL 
    rootContainer->utsContainerListHead = NULL;
    (void)rtems_uts_container_initialize_root(&rootContainer->utsContainer);
#endif

#ifdef RTEMSCFG_MNT_CONTAINER
    rootContainer->mntContainer = NULL; // 初始化mntContainer为NULL 
    rootContainer->mntContainerListHead = NULL;
    (void)rtems_mnt_container_initialize_root(&rootContainer->mntContainer);
#endif

#ifdef RTEMSCFG_NET_CONTAINER
    rootContainer->netContainer = NULL; // 初始化netContainer为NULL
    rootContainer->netContainerListHead = NULL;
    (void)rtems_net_container_initialize_root(&rootContainer->netContainer);
#endif

#ifdef RTEMSCFG_IPC_CONTAINER
    rootContainer->ipcContainer = NULL; // 初始化ipcContainer为NULL
    rootContainer->ipcContainerListHead = NULL;
    (void)rtems_ipc_container_initialize_root(&rootContainer->ipcContainer);
#endif

//         if (idle) {
//             idle->container = rootContainer;
//         }
//     }
// #else
//     Thread_Control *idle = _Per_CPU_Get_by_index(0)->heir;
//     if (idle) {
//         idle->container = rootContainer;
//     }
// #endif

}

Container *rtems_container_get_root(void) {
    return rootContainer;
}

RTEMS_SYSINIT_ITEM(
  rtems_container_initialize_root,           // 初始化函数名
  RTEMS_SYSINIT_BSP_START,
  RTEMS_SYSINIT_ORDER_FIRST
);

// #ifdef RTEMSCFG_PID_CONTAINER
//     struct PidContainer *CurrentPidContainer = NULL;
// #endif

// 之前RTEMS_SYSINIT_ITEM参数
//   RTEMS_SYSINIT_LAST,            // 初始化阶段
//   RTEMS_SYSINIT_ORDER_MIDDLE     // 顺序

// RTEMS_SYSINIT_BSP_START