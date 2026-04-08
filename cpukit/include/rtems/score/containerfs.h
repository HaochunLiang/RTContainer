#ifndef _RTEMS_SCORE_CONTAINERFS_H
#define _RTEMS_SCORE_CONTAINERFS_H

#include <rtems.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ContainerFS: a simple control interface via IMFS files.
 * - PID container control at path "/pidctl" (requires RTEMSCFG_PID_CONTAINER)
 * - UTS container control at path "/utsctl" (requires RTEMSCFG_UTS_CONTAINER)
 * - MNT container control at path "/mntctl" (requires RTEMSCFG_MNT_CONTAINER)
 * - NET container control at path "/netctl" (requires RTEMSCFG_NET_CONTAINER)
 * - IPC container control at path "/ipcctl" (requires RTEMSCFG_IPC_CONTAINER)
 *
 * Build-time switch:
 *   Enabled only when RTEMSCFG_CONTAINER_FILE is defined.
 */

#ifdef RTEMSCFG_CONTAINER_FILE
#ifdef RTEMSCFG_PID_CONTAINER
void rtems_containerfs_register_pidctl(void);
#endif

#ifdef RTEMSCFG_UTS_CONTAINER
void rtems_containerfs_register_utsctl(void);
#endif

#ifdef RTEMSCFG_MNT_CONTAINER
void rtems_containerfs_register_mntctl(void);
#endif

#ifdef RTEMSCFG_NET_CONTAINER
void rtems_containerfs_register_netctl(void);
#endif

#ifdef RTEMSCFG_IPC_CONTAINER
void rtems_containerfs_register_ipcctl(void);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
