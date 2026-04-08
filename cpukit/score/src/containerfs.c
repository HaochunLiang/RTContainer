#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems.h>
#include <rtems/imfs.h>
#include <rtems/libio.h>
#include <rtems/score/container.h>
#include <rtems/score/containerfs.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef RTEMSCFG_PID_CONTAINER
#include <rtems/score/pidContainer.h>
#endif

#ifdef RTEMSCFG_UTS_CONTAINER
#include <rtems/score/utsContainer.h>
#endif

#ifdef RTEMSCFG_MNT_CONTAINER
#include <rtems/score/mntContainer.h>
#endif

#ifdef RTEMSCFG_NET_CONTAINER
#include <rtems/score/netContainer.h>
#endif

#ifdef RTEMSCFG_IPC_CONTAINER
#include <rtems/score/ipcContainer.h>
#endif

#ifdef RTEMSCFG_CONTAINER_FILE

static size_t containerfs_copy_command(char *cmd, size_t cmd_size, const void *buffer, size_t count)
{
  size_t i;
  size_t n;

  if (buffer == NULL || count == 0 || cmd_size == 0) {
    return 0;
  }

  n = count < cmd_size - 1 ? count : cmd_size - 1;
  memcpy(cmd, buffer, n);
  cmd[n] = '\0';

  for (i = 0; i < n; ++i) {
    if (cmd[i] == '\r' || cmd[i] == '\n') {
      cmd[i] = '\0';
      break;
    }
  }

  return n;
}

#ifdef RTEMSCFG_PID_CONTAINER
static ssize_t pidctl_write(rtems_libio_t *iop, const void *buffer, size_t count)
{
  char cmd[128];

  (void) iop;
  if (containerfs_copy_command(cmd, sizeof(cmd), buffer, count) == 0) {
    return (ssize_t) count;
  }

  if (strcmp(cmd, "create") == 0) {
    PidContainer *c = rtems_pid_container_create();
    if (c != NULL) {
      printf("[pidctl] created: id=%d\n", rtems_pid_container_get_id(c));
    } else {
      printf("[pidctl] create failed\n");
    }
  } else if (strncmp(cmd, "delete", 6) == 0) {
    int id;
    Container *root;
    PidContainer *found;
    bool is_root;

    id = -1;
    if (sscanf(cmd + 6, "%d", &id) == 1 && id > 0) {
      root = rtems_container_get_root();
      found = NULL;
      is_root = false;

      if (root != NULL &&
          root->pidContainer != NULL &&
          rtems_pid_container_get_id(root->pidContainer) == id) {
        found = root->pidContainer;
        is_root = true;
      } else if (root != NULL) {
        PidContainerNode *p;

        for (p = root->pidContainerListHead; p != NULL; p = p->next) {
          if (p->pidContainer != NULL && rtems_pid_container_get_id(p->pidContainer) == id) {
            found = p->pidContainer;
            break;
          }
        }
      }

      if (found != NULL) {
        if (is_root) {
          printf("[pidctl] delete: cannot delete root container (id=%d)\n", id);
        } else {
          rtems_pid_container_delete(found);
          printf("[pidctl] deleted: id=%d\n", id);
        }
      } else {
        printf("[pidctl] delete: id=%d not found\n", id);
      }
    } else {
      printf("[pidctl] usage: delete <id>\n");
    }
  } else if (strcmp(cmd, "list") == 0) {
    Container *root;

    root = rtems_container_get_root();
    if (root == NULL || root->pidContainer == NULL) {
      printf("[pidctl] list: no root container\n");
    } else {
      PidContainerNode *p;

      printf("[pidctl] list: root id=%d", rtems_pid_container_get_id(root->pidContainer));
      for (p = root->pidContainerListHead; p != NULL; p = p->next) {
        if (p->pidContainer != NULL) {
          printf(", id=%d", rtems_pid_container_get_id(p->pidContainer));
        }
      }
      printf("\n");
    }
  } else {
    printf("[pidctl] unknown cmd: %s\n", cmd);
    printf("[pidctl] supported: create | delete <id> | list\n");
  }

  return (ssize_t) count;
}

static const rtems_filesystem_file_handlers_r pidctl_handlers = {
  .open_h = rtems_filesystem_default_open,
  .close_h = rtems_filesystem_default_close,
  .read_h = rtems_filesystem_default_read,
  .write_h = pidctl_write,
  .ioctl_h = rtems_filesystem_default_ioctl,
  .lseek_h = rtems_filesystem_default_lseek,
  .fstat_h = rtems_filesystem_default_fstat,
  .ftruncate_h = rtems_filesystem_default_ftruncate,
  .fsync_h = rtems_filesystem_default_fsync_or_fdatasync_success,
  .fdatasync_h = rtems_filesystem_default_fsync_or_fdatasync_success,
  .fcntl_h = rtems_filesystem_default_fcntl,
  .readv_h = rtems_filesystem_default_readv,
  .writev_h = rtems_filesystem_default_writev
};

static const IMFS_node_control pidctl_node_control = {
  .handlers = &pidctl_handlers,
  .node_initialize = IMFS_node_initialize_generic,
  .node_remove = IMFS_node_remove_default,
  .node_destroy = IMFS_node_destroy_default
};

void rtems_containerfs_register_pidctl(void)
{
  static bool created = false;
  int rv;

  if (created) {
    return;
  }

  rv = IMFS_make_generic_node(
    "/pidctl",
    S_IFCHR | S_IRWXU | S_IRWXG | S_IRWXO,
    &pidctl_node_control,
    NULL
  );
  if (rv == 0) {
    created = true;
    printf("[pidctl] control file registered at /pidctl\n");
  } else {
    printf("[pidctl] failed to register control file (rv=%d)\n", rv);
  }
}
#else
void rtems_containerfs_register_pidctl(void)
{
}
#endif

#ifdef RTEMSCFG_UTS_CONTAINER
static ssize_t utsctl_write(rtems_libio_t *iop, const void *buffer, size_t count)
{
  char cmd[256];

  (void) iop;
  if (containerfs_copy_command(cmd, sizeof(cmd), buffer, count) == 0) {
    return (ssize_t) count;
  }

  if (strncmp(cmd, "create", 6) == 0) {
    const char *name = NULL;
    UtsContainer *c;

    if (cmd[6] == ' ') {
      name = cmd + 7;
      while (*name == ' ') {
        ++name;
      }
      if (*name == '\0') {
        name = NULL;
      }
    }

    c = rtems_uts_container_create(name);
    if (c != NULL) {
      printf("[utsctl] created: id=%d, name=%s\n", c->ID, c->name);
    } else {
      printf("[utsctl] create failed\n");
    }
  } else if (strncmp(cmd, "delete", 6) == 0) {
    int id;
    Container *root;
    UtsContainer *found;
    bool is_root;

    id = -1;
    if (sscanf(cmd + 6, "%d", &id) == 1 && id > 0) {
      root = rtems_container_get_root();
      found = NULL;
      is_root = false;

      if (root != NULL && root->utsContainer != NULL && root->utsContainer->ID == id) {
        found = root->utsContainer;
        is_root = true;
      } else if (root != NULL) {
        UtsContainerNode *p;

        for (p = root->utsContainerListHead; p != NULL; p = p->next) {
          if (p->utsContainer != NULL && p->utsContainer->ID == id) {
            found = p->utsContainer;
            break;
          }
        }
      }

      if (found != NULL) {
        if (is_root) {
          printf("[utsctl] delete: cannot delete root container (id=%d)\n", id);
        } else {
          rtems_uts_container_delete(found);
          printf("[utsctl] deleted: id=%d\n", id);
        }
      } else {
        printf("[utsctl] delete: id=%d not found\n", id);
      }
    } else {
      printf("[utsctl] usage: delete <id>\n");
    }
  } else if (strcmp(cmd, "list") == 0) {
    Container *root;

    root = rtems_container_get_root();
    if (root == NULL || root->utsContainer == NULL) {
      printf("[utsctl] list: no root container\n");
    } else {
      UtsContainerNode *p;

      printf("[utsctl] list: root id=%d name=%s", root->utsContainer->ID, root->utsContainer->name);
      for (p = root->utsContainerListHead; p != NULL; p = p->next) {
        if (p->utsContainer != NULL) {
          printf(", id=%d name=%s", p->utsContainer->ID, p->utsContainer->name);
        }
      }
      printf("\n");
    }
  } else {
    printf("[utsctl] unknown cmd: %s\n", cmd);
    printf("[utsctl] supported: create [name] | delete <id> | list\n");
  }

  return (ssize_t) count;
}

static const rtems_filesystem_file_handlers_r utsctl_handlers = {
  .open_h = rtems_filesystem_default_open,
  .close_h = rtems_filesystem_default_close,
  .read_h = rtems_filesystem_default_read,
  .write_h = utsctl_write,
  .ioctl_h = rtems_filesystem_default_ioctl,
  .lseek_h = rtems_filesystem_default_lseek,
  .fstat_h = rtems_filesystem_default_fstat,
  .ftruncate_h = rtems_filesystem_default_ftruncate,
  .fsync_h = rtems_filesystem_default_fsync_or_fdatasync_success,
  .fdatasync_h = rtems_filesystem_default_fsync_or_fdatasync_success,
  .fcntl_h = rtems_filesystem_default_fcntl,
  .readv_h = rtems_filesystem_default_readv,
  .writev_h = rtems_filesystem_default_writev
};

static const IMFS_node_control utsctl_node_control = {
  .handlers = &utsctl_handlers,
  .node_initialize = IMFS_node_initialize_generic,
  .node_remove = IMFS_node_remove_default,
  .node_destroy = IMFS_node_destroy_default
};

void rtems_containerfs_register_utsctl(void)
{
  static bool created = false;
  int rv;

  if (created) {
    return;
  }

  rv = IMFS_make_generic_node(
    "/utsctl",
    S_IFCHR | S_IRWXU | S_IRWXG | S_IRWXO,
    &utsctl_node_control,
    NULL
  );
  if (rv == 0) {
    created = true;
    printf("[utsctl] control file registered at /utsctl\n");
  } else {
    printf("[utsctl] failed to register control file (rv=%d)\n", rv);
  }
}
#else
void rtems_containerfs_register_utsctl(void)
{
}
#endif

#ifdef RTEMSCFG_MNT_CONTAINER
static ssize_t mntctl_write(rtems_libio_t *iop, const void *buffer, size_t count)
{
  char cmd[256];

  (void) iop;
  if (containerfs_copy_command(cmd, sizeof(cmd), buffer, count) == 0) {
    return (ssize_t) count;
  }

  if (strcmp(cmd, "create") == 0) {
    MntContainer *c = rtems_mnt_container_create();
    if (c != NULL) {
      rtems_mnt_container_add_to_list(c);
      printf("[mntctl] created: id=%d\n", rtems_mnt_container_get_id(c));
    } else {
      printf("[mntctl] create failed\n");
    }
  } else if (strncmp(cmd, "create_inherit", 14) == 0) {
    int parent_id;

    parent_id = -1;
    if (sscanf(cmd + 14, "%d", &parent_id) == 1 && parent_id > 0) {
      Container *root;
      MntContainer *parent;

      root = rtems_container_get_root();
      parent = NULL;

      if (root != NULL &&
          root->mntContainer != NULL &&
          rtems_mnt_container_get_id(root->mntContainer) == parent_id) {
        parent = root->mntContainer;
      } else if (root != NULL) {
        MntContainerNode *p;

        for (p = root->mntContainerListHead; p != NULL; p = p->next) {
          if (p->mntContainer != NULL && rtems_mnt_container_get_id(p->mntContainer) == parent_id) {
            parent = p->mntContainer;
            break;
          }
        }
      }

      if (parent != NULL) {
        MntContainer *c = rtems_mnt_container_create_with_inheritance(parent);
        if (c != NULL) {
          rtems_mnt_container_add_to_list(c);
          printf(
            "[mntctl] created with inheritance: id=%d, parent_id=%d\n",
            rtems_mnt_container_get_id(c),
            parent_id
          );
        } else {
          printf("[mntctl] create with inheritance failed\n");
        }
      } else {
        printf("[mntctl] parent container id=%d not found\n", parent_id);
      }
    } else {
      printf("[mntctl] usage: create_inherit <parent_id>\n");
    }
  } else if (strncmp(cmd, "delete", 6) == 0) {
    int id;
    Container *root;
    MntContainer *found;
    bool is_root;

    id = -1;
    if (sscanf(cmd + 6, "%d", &id) == 1 && id > 0) {
      root = rtems_container_get_root();
      found = NULL;
      is_root = false;

      if (root != NULL &&
          root->mntContainer != NULL &&
          rtems_mnt_container_get_id(root->mntContainer) == id) {
        found = root->mntContainer;
        is_root = true;
      } else if (root != NULL) {
        MntContainerNode *p;

        for (p = root->mntContainerListHead; p != NULL; p = p->next) {
          if (p->mntContainer != NULL && rtems_mnt_container_get_id(p->mntContainer) == id) {
            found = p->mntContainer;
            break;
          }
        }
      }

      if (found != NULL) {
        if (is_root) {
          printf("[mntctl] delete: cannot delete root container (id=%d)\n", id);
        } else {
          rtems_mnt_container_delete(found);
          printf("[mntctl] deleted: id=%d\n", id);
        }
      } else {
        printf("[mntctl] delete: id=%d not found\n", id);
      }
    } else {
      printf("[mntctl] usage: delete <id>\n");
    }
  } else if (strcmp(cmd, "list") == 0) {
    Container *root;

    root = rtems_container_get_root();
    if (root == NULL || root->mntContainer == NULL) {
      printf("[mntctl] list: no root container\n");
    } else {
      MntContainerNode *p;

      printf("[mntctl] list: root id=%d", rtems_mnt_container_get_id(root->mntContainer));
      for (p = root->mntContainerListHead; p != NULL; p = p->next) {
        if (p->mntContainer != NULL) {
          printf(", id=%d", rtems_mnt_container_get_id(p->mntContainer));
        }
      }
      printf("\n");
    }
  } else {
    printf("[mntctl] unknown cmd: %s\n", cmd);
    printf("[mntctl] supported: create | create_inherit <parent_id> | delete <id> | list\n");
  }

  return (ssize_t) count;
}

static const rtems_filesystem_file_handlers_r mntctl_handlers = {
  .open_h = rtems_filesystem_default_open,
  .close_h = rtems_filesystem_default_close,
  .read_h = rtems_filesystem_default_read,
  .write_h = mntctl_write,
  .ioctl_h = rtems_filesystem_default_ioctl,
  .lseek_h = rtems_filesystem_default_lseek,
  .fstat_h = rtems_filesystem_default_fstat,
  .ftruncate_h = rtems_filesystem_default_ftruncate,
  .fsync_h = rtems_filesystem_default_fsync_or_fdatasync_success,
  .fdatasync_h = rtems_filesystem_default_fsync_or_fdatasync_success,
  .fcntl_h = rtems_filesystem_default_fcntl,
  .readv_h = rtems_filesystem_default_readv,
  .writev_h = rtems_filesystem_default_writev
};

static const IMFS_node_control mntctl_node_control = {
  .handlers = &mntctl_handlers,
  .node_initialize = IMFS_node_initialize_generic,
  .node_remove = IMFS_node_remove_default,
  .node_destroy = IMFS_node_destroy_default
};

void rtems_containerfs_register_mntctl(void)
{
  static bool created = false;
  int rv;

  if (created) {
    return;
  }

  rv = IMFS_make_generic_node(
    "/mntctl",
    S_IFCHR | S_IRWXU | S_IRWXG | S_IRWXO,
    &mntctl_node_control,
    NULL
  );
  if (rv == 0) {
    created = true;
    printf("[mntctl] control file registered at /mntctl\n");
  } else {
    printf("[mntctl] failed to register control file (rv=%d)\n", rv);
  }
}
#else
void rtems_containerfs_register_mntctl(void)
{
}
#endif

#ifdef RTEMSCFG_NET_CONTAINER
static ssize_t netctl_write(rtems_libio_t *iop, const void *buffer, size_t count)
{
  char cmd[256];

  (void) iop;
  if (containerfs_copy_command(cmd, sizeof(cmd), buffer, count) == 0) {
    return (ssize_t) count;
  }

  if (strcmp(cmd, "create") == 0) {
    NetContainer *c = rtems_net_container_create();
    if (c != NULL) {
      printf("[netctl] created: id=%d\n", rtems_net_container_get_id(c));
    } else {
      printf("[netctl] create failed\n");
    }
  } else if (strncmp(cmd, "delete", 6) == 0) {
    int id;
    Container *root;
    NetContainer *found;
    bool is_root;

    id = -1;
    if (sscanf(cmd + 6, "%d", &id) == 1 && id > 0) {
      root = rtems_container_get_root();
      found = NULL;
      is_root = false;

      if (root != NULL &&
          root->netContainer != NULL &&
          rtems_net_container_get_id(root->netContainer) == id) {
        found = root->netContainer;
        is_root = true;
      } else if (root != NULL) {
        NetContainerNode *p;

        for (p = root->netContainerListHead; p != NULL; p = p->next) {
          if (p->netContainer != NULL && rtems_net_container_get_id(p->netContainer) == id) {
            found = p->netContainer;
            break;
          }
        }
      }

      if (found != NULL) {
        if (is_root) {
          printf("[netctl] delete: cannot delete root container (id=%d)\n", id);
        } else {
          rtems_net_container_delete(found);
          printf("[netctl] deleted: id=%d\n", id);
        }
      } else {
        printf("[netctl] delete: id=%d not found\n", id);
      }
    } else {
      printf("[netctl] usage: delete <id>\n");
    }
  } else if (strcmp(cmd, "list") == 0) {
    Container *root;

    root = rtems_container_get_root();
    if (root == NULL || root->netContainer == NULL) {
      printf("[netctl] list: no root container\n");
    } else {
      NetContainerNode *p;

      printf("[netctl] list: root id=%d", rtems_net_container_get_id(root->netContainer));
      for (p = root->netContainerListHead; p != NULL; p = p->next) {
        if (p->netContainer != NULL) {
          printf(", id=%d", rtems_net_container_get_id(p->netContainer));
        }
      }
      printf("\n");
    }
  } else {
    printf("[netctl] unknown cmd: %s\n", cmd);
    printf("[netctl] supported: create | delete <id> | list\n");
  }

  return (ssize_t) count;
}

static const rtems_filesystem_file_handlers_r netctl_handlers = {
  .open_h = rtems_filesystem_default_open,
  .close_h = rtems_filesystem_default_close,
  .read_h = rtems_filesystem_default_read,
  .write_h = netctl_write,
  .ioctl_h = rtems_filesystem_default_ioctl,
  .lseek_h = rtems_filesystem_default_lseek,
  .fstat_h = rtems_filesystem_default_fstat,
  .ftruncate_h = rtems_filesystem_default_ftruncate,
  .fsync_h = rtems_filesystem_default_fsync_or_fdatasync_success,
  .fdatasync_h = rtems_filesystem_default_fsync_or_fdatasync_success,
  .fcntl_h = rtems_filesystem_default_fcntl,
  .readv_h = rtems_filesystem_default_readv,
  .writev_h = rtems_filesystem_default_writev
};

static const IMFS_node_control netctl_node_control = {
  .handlers = &netctl_handlers,
  .node_initialize = IMFS_node_initialize_generic,
  .node_remove = IMFS_node_remove_default,
  .node_destroy = IMFS_node_destroy_default
};

void rtems_containerfs_register_netctl(void)
{
  static bool created = false;
  int rv;

  if (created) {
    return;
  }

  rv = IMFS_make_generic_node(
    "/netctl",
    S_IFCHR | S_IRWXU | S_IRWXG | S_IRWXO,
    &netctl_node_control,
    NULL
  );
  if (rv == 0) {
    created = true;
    printf("[netctl] control file registered at /netctl\n");
  } else {
    printf("[netctl] failed to register control file (rv=%d)\n", rv);
  }
}
#else
void rtems_containerfs_register_netctl(void)
{
}
#endif

#ifdef RTEMSCFG_IPC_CONTAINER
static ssize_t ipcctl_write(rtems_libio_t *iop, const void *buffer, size_t count)
{
  char cmd[256];

  (void) iop;
  if (containerfs_copy_command(cmd, sizeof(cmd), buffer, count) == 0) {
    return (ssize_t) count;
  }

  if (strcmp(cmd, "create") == 0) {
    IpcContainer *c = rtems_ipc_container_create();
    if (c != NULL) {
      printf("[ipcctl] created: id=%d\n", rtems_ipc_container_get_id(c));
    } else {
      printf("[ipcctl] create failed\n");
    }
  } else if (strncmp(cmd, "delete", 6) == 0) {
    int id;
    Container *root;
    IpcContainer *found;
    bool is_root;

    id = -1;
    if (sscanf(cmd + 6, "%d", &id) == 1 && id > 0) {
      root = rtems_container_get_root();
      found = NULL;
      is_root = false;

      if (root != NULL &&
          root->ipcContainer != NULL &&
          rtems_ipc_container_get_id(root->ipcContainer) == id) {
        found = root->ipcContainer;
        is_root = true;
      } else if (root != NULL) {
        IpcContainerNode *p;

        for (p = root->ipcContainerListHead; p != NULL; p = p->next) {
          if (p->ipcContainer != NULL && rtems_ipc_container_get_id(p->ipcContainer) == id) {
            found = p->ipcContainer;
            break;
          }
        }
      }

      if (found != NULL) {
        if (is_root) {
          printf("[ipcctl] delete: cannot delete root container (id=%d)\n", id);
        } else {
          rtems_ipc_container_delete(found);
          printf("[ipcctl] deleted: id=%d\n", id);
        }
      } else {
        printf("[ipcctl] delete: id=%d not found\n", id);
      }
    } else {
      printf("[ipcctl] usage: delete <id>\n");
    }
  } else if (strcmp(cmd, "list") == 0) {
    Container *root;

    root = rtems_container_get_root();
    if (root == NULL || root->ipcContainer == NULL) {
      printf("[ipcctl] list: no root container\n");
    } else {
      IpcContainerNode *p;

      printf("[ipcctl] list: root id=%d", rtems_ipc_container_get_id(root->ipcContainer));
      for (p = root->ipcContainerListHead; p != NULL; p = p->next) {
        if (p->ipcContainer != NULL) {
          printf(", id=%d", rtems_ipc_container_get_id(p->ipcContainer));
        }
      }
      printf("\n");
    }
  } else {
    printf("[ipcctl] unknown cmd: %s\n", cmd);
    printf("[ipcctl] supported: create | delete <id> | list\n");
  }

  return (ssize_t) count;
}

static const rtems_filesystem_file_handlers_r ipcctl_handlers = {
  .open_h = rtems_filesystem_default_open,
  .close_h = rtems_filesystem_default_close,
  .read_h = rtems_filesystem_default_read,
  .write_h = ipcctl_write,
  .ioctl_h = rtems_filesystem_default_ioctl,
  .lseek_h = rtems_filesystem_default_lseek,
  .fstat_h = rtems_filesystem_default_fstat,
  .ftruncate_h = rtems_filesystem_default_ftruncate,
  .fsync_h = rtems_filesystem_default_fsync_or_fdatasync_success,
  .fdatasync_h = rtems_filesystem_default_fsync_or_fdatasync_success,
  .fcntl_h = rtems_filesystem_default_fcntl,
  .readv_h = rtems_filesystem_default_readv,
  .writev_h = rtems_filesystem_default_writev
};

static const IMFS_node_control ipcctl_node_control = {
  .handlers = &ipcctl_handlers,
  .node_initialize = IMFS_node_initialize_generic,
  .node_remove = IMFS_node_remove_default,
  .node_destroy = IMFS_node_destroy_default
};

void rtems_containerfs_register_ipcctl(void)
{
  static bool created = false;
  int rv;

  if (created) {
    return;
  }

  rv = IMFS_make_generic_node(
    "/ipcctl",
    S_IFCHR | S_IRWXU | S_IRWXG | S_IRWXO,
    &ipcctl_node_control,
    NULL
  );
  if (rv == 0) {
    created = true;
    printf("[ipcctl] control file registered at /ipcctl\n");
  } else {
    printf("[ipcctl] failed to register control file (rv=%d)\n", rv);
  }
}
#else
void rtems_containerfs_register_ipcctl(void)
{
}
#endif

#else
void rtems_containerfs_register_pidctl(void)
{
}

void rtems_containerfs_register_utsctl(void)
{
}

void rtems_containerfs_register_mntctl(void)
{
}

void rtems_containerfs_register_netctl(void)
{
}

void rtems_containerfs_register_ipcctl(void)
{
}
#endif
