/* SPDX-License-Identifier: BSD-2-Clause */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../support.h"
#include <rtems/io_cgroup.h>
#include <rtems/libcsupport.h>
#include <rtems/rtems/cgroup.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/score/container.h>
#include <rtems/score/objectimpl.h>
#include <rtems/score/threadimpl.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const char rtems_test_name[] = "CONTAINER ABNORMAL 01";

typedef enum {
  FAIL_NONE, FAIL_PID, FAIL_UTS, FAIL_MNT, FAIL_NET, FAIL_IPC,
  FAIL_CGROUP, FAIL_IO
} FailureStage;

static FailureStage failure_stage;
static unsigned injection_hits;
static rtems_id created_cgroup_id;
static rtems_id root_queue;
static int root_pid_references;
static Objects_Information *initial_queue_info;
static Objects_Information *initial_semaphore_info;

static bool inject(FailureStage stage)
{
  if (failure_stage == stage) {
    ++injection_hits;
    return true;
  }
  return false;
}

/* Linker wrappers affect only this executable. Real rollback is never wrapped. */
PidContainer *__real_rtems_pid_container_create(void);
PidContainer *__wrap_rtems_pid_container_create(void)
{
  return inject(FAIL_PID) ? NULL : __real_rtems_pid_container_create();
}

UtsContainer *__real_rtems_uts_container_create(const char *name);
UtsContainer *__wrap_rtems_uts_container_create(const char *name)
{
  return inject(FAIL_UTS) ? NULL : __real_rtems_uts_container_create(name);
}

MntContainer *__real_rtems_mnt_container_create_with_inheritance(MntContainer *p);
MntContainer *__wrap_rtems_mnt_container_create_with_inheritance(MntContainer *p)
{
  return inject(FAIL_MNT) ? NULL :
    __real_rtems_mnt_container_create_with_inheritance(p);
}

NetContainer *__real_rtems_net_container_create(void);
NetContainer *__wrap_rtems_net_container_create(void)
{
  return inject(FAIL_NET) ? NULL : __real_rtems_net_container_create();
}

IpcContainer *__real_rtems_ipc_container_create(void);
IpcContainer *__wrap_rtems_ipc_container_create(void)
{
  return inject(FAIL_IPC) ? NULL : __real_rtems_ipc_container_create();
}

rtems_status_code __real_rtems_cgroup_create(
  rtems_name name, rtems_id *id, CORE_cgroup_config *config
);
rtems_status_code __wrap_rtems_cgroup_create(
  rtems_name name, rtems_id *id, CORE_cgroup_config *config
)
{
  rtems_status_code sc;
  if (inject(FAIL_CGROUP)) {
    *id = 0;
    return RTEMS_TOO_MANY;
  }
  sc = __real_rtems_cgroup_create(name, id, config);
  if (sc == RTEMS_SUCCESSFUL) {
    created_cgroup_id = *id;
  }
  return sc;
}

rtems_status_code __real_rtems_io_cgroup_create(
  uint16_t weight, const IO_Cgroup_Limit *limits, uint32_t *id
);
rtems_status_code __wrap_rtems_io_cgroup_create(
  uint16_t weight, const IO_Cgroup_Limit *limits, uint32_t *id
)
{
  if (inject(FAIL_IO)) {
    *id = 0;
    return RTEMS_NO_MEMORY;
  }
  return __real_rtems_io_cgroup_create(weight, limits, id);
}

static void count_io(IO_Cgroup_Control *io, void *arg)
{
  (void) io;
  ++*(unsigned *) arg;
}

static unsigned io_count(void)
{
  unsigned count = 0;
  rtems_io_cgroup_traverse(count_io, &count);
  return count;
}

static uint32_t flags_for_failure(FailureStage stage)
{
  uint32_t flags = RTEMS_UNIFIED_CONTAINER_PID;

  if (stage >= FAIL_UTS) flags |= RTEMS_UNIFIED_CONTAINER_UTS;
  if (stage >= FAIL_MNT) flags |= RTEMS_UNIFIED_CONTAINER_MNT;
  if (stage >= FAIL_NET) flags |= RTEMS_UNIFIED_CONTAINER_NET;
  if (stage >= FAIL_IPC) flags |= RTEMS_UNIFIED_CONTAINER_IPC;
  if (stage >= FAIL_CGROUP) flags |= RTEMS_UNIFIED_CONTAINER_CPU;
  if (stage >= FAIL_IO) flags |= RTEMS_UNIFIED_CONTAINER_IO;
  return flags;
}

static void check_heap(const char *phase)
{
  if (!malloc_walk(0, false)) {
    printf("[FAIL] heap corruption after %s\n", phase);
    rtems_test_assert(false);
  }
}

static void check_network_sockets(void)
{
  static const int types[] = { SOCK_DGRAM, SOCK_STREAM };
  size_t i;

  /* Both protocols must be usable after repeated NET allocation/rollback.
   * Opening a socket inserts its PCB into the container's hash table. */
  for (i = 0; i < RTEMS_ARRAY_SIZE(types); ++i) {
    int fd = socket(AF_INET, types[i], 0);

    rtems_test_assert(fd >= 0);
    rtems_test_assert(close(fd) == 0);
  }
  check_heap("UDP/TCP socket creation and close");
}

static void check_resources(const rtems_resource_snapshot *before)
{
  rtems_resource_snapshot after;

  rtems_resource_snapshot_take(&after);
  if (!rtems_resource_snapshot_equal(before, &after)) {
    printf("[resources] heap used: %" PRIuPTR " -> %" PRIuPTR
      ", free: %" PRIuPTR " -> %" PRIuPTR "\n",
      (uintptr_t) before->heap_info.Used.total,
      (uintptr_t) after.heap_info.Used.total,
      (uintptr_t) before->heap_info.Free.total,
      (uintptr_t) after.heap_info.Free.total);
    printf("[resources] workspace used: %" PRIuPTR " -> %" PRIuPTR
      ", free: %" PRIuPTR " -> %" PRIuPTR "\n",
      (uintptr_t) before->workspace_info.Used.total,
      (uintptr_t) after.workspace_info.Used.total,
      (uintptr_t) before->workspace_info.Free.total,
      (uintptr_t) after.workspace_info.Free.total);
    printf("[resources] tasks: %" PRIu32 " -> %" PRIu32
      ", semaphores: %" PRIu32 " -> %" PRIu32 ", files: %d -> %d\n",
      before->rtems_api.active_tasks, after.rtems_api.active_tasks,
      before->rtems_api.active_semaphores, after.rtems_api.active_semaphores,
      before->open_files, after.open_files);
    printf("[resources] rtems objects: barriers %" PRIu32 "/%" PRIu32
      ", extensions %" PRIu32 "/%" PRIu32 ", queues %" PRIu32 "/%" PRIu32
      ", timers %" PRIu32 "/%" PRIu32 "\n",
      before->rtems_api.active_barriers, after.rtems_api.active_barriers,
      before->rtems_api.active_extensions, after.rtems_api.active_extensions,
      before->rtems_api.active_message_queues, after.rtems_api.active_message_queues,
      before->rtems_api.active_timers, after.rtems_api.active_timers);
    printf("[resources] posix objects: queues %" PRIu32 "/%" PRIu32
      ", semaphores %" PRIu32 "/%" PRIu32 ", threads %" PRIu32 "/%" PRIu32
      ", keys %" PRIu32 "/%" PRIu32 ", key-values %" PRIu32 "/%" PRIu32 "\n",
      before->posix_api.active_message_queues, after.posix_api.active_message_queues,
      before->posix_api.active_semaphores, after.posix_api.active_semaphores,
      before->posix_api.active_threads, after.posix_api.active_threads,
      before->active_posix_keys, after.active_posix_keys,
      before->active_posix_key_value_pairs, after.active_posix_key_value_pairs);
  }
  /* Check allocated memory and files for both rollback and recovery.
   * Per-class object counts are reported separately for diagnosis. */
  CHECK(before->heap_info.Used.total == after.heap_info.Used.total);
  CHECK(before->heap_info.Free.total == after.heap_info.Free.total);
  CHECK(before->workspace_info.Used.total == after.workspace_info.Used.total);
  CHECK(before->workspace_info.Free.total == after.workspace_info.Free.total);
  CHECK(before->open_files == after.open_files);
}

static void check_object_registry(void)
{
  /* Check pointer identity before a snapshot can dereference a stale child
   * entry.  Startup may register static pools after creating the root IPC
   * namespace, so preserve the entries observed before the first child. */
  rtems_test_assert(
    _Objects_Information_table[OBJECTS_CLASSIC_API]
      [OBJECTS_RTEMS_MESSAGE_QUEUES] == initial_queue_info
  );
  rtems_test_assert(
    _Objects_Information_table[OBJECTS_CLASSIC_API]
      [OBJECTS_RTEMS_SEMAPHORES] == initial_semaphore_info
  );
}

static void check_root(const Container *before)
{
  Container *root = rtems_container_get_root();
  Thread_Control *self = _Thread_Get_executing();
  uint32_t sent = 0x1234abcd;
  uint32_t received = 0;
  size_t size = 0;
  char hostname[32];

  CHECK(root->pidContainer == before->pidContainer);
  CHECK(root->utsContainer == before->utsContainer);
  CHECK(root->mntContainer == before->mntContainer);
  CHECK(root->netContainer == before->netContainer);
  CHECK(root->ipcContainer == before->ipcContainer);
  /* Head identity also detects partially created namespace registrations. */
  CHECK(root->pidContainerListHead == before->pidContainerListHead);
  CHECK(root->utsContainerListHead == before->utsContainerListHead);
  CHECK(root->mntContainerListHead == before->mntContainerListHead);
  CHECK(root->netContainerListHead == before->netContainerListHead);
  CHECK(root->ipcContainerListHead == before->ipcContainerListHead);
  CHECK(self->container->pidContainer == root->pidContainer);
  CHECK(rtems_pid_container_find_by_thread(self) == root->pidContainer);
  CHECK(rtems_pid_container_get_rc(root->pidContainer) == root_pid_references);
  CHECK(self->container->utsContainer == root->utsContainer);
  CHECK(self->container->mntContainer == root->mntContainer);
  CHECK(self->container->netContainer == root->netContainer);
  CHECK(self->container->ipcContainer == root->ipcContainer);
  CHECK(self->cgroup == NULL);
  rtems_test_assert(gethostname(hostname, sizeof(hostname)) == 0);
  CHECK(strcmp(hostname, "abnormal-root") == 0);
  rtems_test_assert(rtems_message_queue_send(
    root_queue, &sent, sizeof(sent)
  ) == RTEMS_SUCCESSFUL);
  rtems_test_assert(rtems_message_queue_receive(
    root_queue, &received, &size, RTEMS_NO_WAIT, 0
  ) == RTEMS_SUCCESSFUL);
  CHECK(size == sizeof(sent) && received == sent);
  check_object_registry();
}

static rtems_task Init(rtems_task_argument arg)
{
  static const char *const names[] = {
    "none", "PID", "UTS", "MNT", "NET", "IPC", "cgroup", "IO cgroup"
  };
  RtemsContainerConfig config;
  RtemsContainer *container;
  Container root_before;
  rtems_resource_snapshot resources;
  unsigned baseline_io;
  unsigned round;
  FailureStage stage;
  rtems_status_code sc;
  uint32_t count;

  (void) arg;
  abnormal_begin();
  rtems_test_assert(rtems_bsdnet_initialize_network() == 0);
  rtems_test_assert(sethostname("abnormal-root", 13) == 0);
  rtems_test_assert(rtems_message_queue_create(
    rtems_build_name('R', 'O', 'O', 'T'), 1, sizeof(uint32_t),
    RTEMS_DEFAULT_ATTRIBUTES, &root_queue
  ) == RTEMS_SUCCESSFUL);
  rtems_unified_container_config_initialize(&config);
  config.flags = RTEMS_UNIFIED_CONTAINER_ALL;
  initial_queue_info = _Objects_Information_table[OBJECTS_CLASSIC_API]
    [OBJECTS_RTEMS_MESSAGE_QUEUES];
  initial_semaphore_info = _Objects_Information_table[OBJECTS_CLASSIC_API]
    [OBJECTS_RTEMS_SEMAPHORES];

  /* Warm up lazy allocations, including the task's own namespace context,
   * before checking both rollback and complete recovery for leaks. */
  rtems_test_assert(rtems_unified_container_create(
    &config, &container
  ) == RTEMS_SUCCESSFUL);
  check_object_registry();
  rtems_test_assert(rtems_unified_container_enter(
    container, _Thread_Get_executing()
  ) == RTEMS_SUCCESSFUL);
  check_network_sockets();
  rtems_test_assert(rtems_unified_container_leave(
    container, _Thread_Get_executing()
  ) == RTEMS_SUCCESSFUL);
  rtems_test_assert(rtems_unified_container_delete(container) == RTEMS_SUCCESSFUL);
  check_heap("warm-up cleanup");
  root_before = *rtems_container_get_root();
  root_pid_references = rtems_pid_container_get_rc(root_before.pidContainer);
  check_root(&root_before);
  baseline_io = io_count();

  for (round = 0; round < 3; ++round) {
    for (stage = FAIL_PID; stage <= FAIL_IO; ++stage) {
      printf("[inject] round=%u stage=%s\n", round + 1, names[stage]);
      rtems_resource_snapshot_take(&resources);
      created_cgroup_id = 0;
      injection_hits = 0;
      failure_stage = stage;
      config.flags = flags_for_failure(stage);
      /* A non-NULL sentinel verifies that create clears its output on error. */
      container = (RtemsContainer *) &config;
      sc = rtems_unified_container_create(&config, &container);
      failure_stage = FAIL_NONE;
      check_heap("injected creation failure and rollback");
      CHECK(injection_hits == 1);
      CHECK(sc == (stage == FAIL_CGROUP ? RTEMS_TOO_MANY : RTEMS_NO_MEMORY));
      CHECK(container == NULL);
      CHECK(io_count() == baseline_io);
      if (created_cgroup_id != 0) {
        CHECK(rtems_cgroup_get_task_count(created_cgroup_id, &count) == RTEMS_INVALID_ID);
      }
      check_root(&root_before);
      check_resources(&resources);

      puts("[recovery] creating container");
      rtems_resource_snapshot_take(&resources);
      rtems_test_assert(rtems_unified_container_create(
        &config, &container
      ) == RTEMS_SUCCESSFUL);
      check_object_registry();
      check_heap("recovery creation");
      rtems_test_assert(container != NULL);
      puts("[recovery] entering container");
      rtems_test_assert(rtems_unified_container_enter(
        container, _Thread_Get_executing()
      ) == RTEMS_SUCCESSFUL);
      if ((config.flags & RTEMS_UNIFIED_CONTAINER_NET) != 0) {
        check_network_sockets();
      }
      puts("[recovery] leaving container");
      rtems_test_assert(rtems_unified_container_leave(
        container, _Thread_Get_executing()
      ) == RTEMS_SUCCESSFUL);
      puts("[recovery] deleting container");
      rtems_test_assert(rtems_unified_container_delete(container) == RTEMS_SUCCESSFUL);
      check_heap("recovery cleanup");
      check_root(&root_before);
      CHECK(io_count() == baseline_io);
      check_resources(&resources);
      puts("[recovery] create, enter, leave and delete completed");
    }
  }
  rtems_test_assert(rtems_message_queue_delete(root_queue) == RTEMS_SUCCESSFUL);
  abnormal_finish();
}

struct rtems_bsdnet_config rtems_bsdnet_config = {
  NULL, NULL, 0, 0, 0, 0, 0, 0, 0,
  {"0.0.0.0"}, {"0.0.0.0"}, 0, 0, 0, 0, 0
};

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_APPLICATION_NEEDS_LIBNETWORKING
#define CONFIGURE_MAXIMUM_TASKS 8
#define CONFIGURE_MAXIMUM_CGROUPS 1
#define CONFIGURE_MAXIMUM_SEMAPHORES 12
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 1
#define CONFIGURE_MESSAGE_BUFFER_MEMORY \
  CONFIGURE_MESSAGE_BUFFERS_FOR_QUEUE(1, sizeof(uint32_t))
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 20
#define CONFIGURE_EXECUTIVE_RAM_SIZE (16 * 1024 * 1024)
#define CONFIGURE_INIT_TASK_PRIORITY 10
#define CONFIGURE_INIT_TASK_STACK_SIZE (4 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
