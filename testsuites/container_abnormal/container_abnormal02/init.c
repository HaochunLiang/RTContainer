/* SPDX-License-Identifier: BSD-2-Clause */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../support.h"
#include <rtems/rtems/cgroup.h>
#include <rtems/score/container.h>
#include <rtems/score/containerfs.h>
#include <rtems/score/threadimpl.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

const char rtems_test_name[] = "CONTAINER ABNORMAL 02";

#define PROBE RTEMS_EVENT_0
#define STOP RTEMS_EVENT_1
#define READY(i) (RTEMS_EVENT_0 << (i))
#define ACK(i) (RTEMS_EVENT_2 << (i))
#define DONE(i) (RTEMS_EVENT_4 << (i))

static rtems_id manager_id;
static rtems_id worker_ids[2];
static RtemsContainer *containers[2];
static int control_fd;

static void receive_event(rtems_event_set event)
{
  rtems_event_set received;
  rtems_test_assert(rtems_event_receive(
    event, RTEMS_EVENT_ALL | RTEMS_WAIT,
    2 * rtems_clock_get_ticks_per_second(), &received
  ) == RTEMS_SUCCESSFUL);
}

static rtems_task worker(rtems_task_argument index)
{
  rtems_event_set received;
  rtems_test_assert(rtems_unified_container_enter(
    containers[index], _Thread_Get_executing()
  ) == RTEMS_SUCCESSFUL);
  rtems_test_assert(rtems_event_send(manager_id, READY(index)) == RTEMS_SUCCESSFUL);
  for (;;) {
    rtems_test_assert(rtems_event_receive(
      PROBE | STOP, RTEMS_EVENT_ANY | RTEMS_WAIT,
      20 * rtems_clock_get_ticks_per_second(), &received
    ) == RTEMS_SUCCESSFUL);
    if ((received & STOP) != 0) {
      break;
    }
    rtems_test_assert(rtems_event_send(manager_id, ACK(index)) == RTEMS_SUCCESSFUL);
  }
  rtems_test_assert(rtems_unified_container_leave(
    containers[index], _Thread_Get_executing()
  ) == RTEMS_SUCCESSFUL);
  rtems_test_assert(rtems_event_send(manager_id, DONE(index)) == RTEMS_SUCCESSFUL);
  rtems_task_exit();
}

static void probe(unsigned index)
{
  rtems_test_assert(rtems_event_send(worker_ids[index], PROBE) == RTEMS_SUCCESSFUL);
  receive_event(ACK(index));
}

static void command(const char *text, bool accepted)
{
  ssize_t n;
  int error;
  errno = 0;
  n = write(control_fd, text, strlen(text));
  error = errno;
  printf("[command] %s => write=%zd errno=%d\n", text, n, error);
  if (accepted) {
    rtems_test_assert(n == (ssize_t) strlen(text));
  } else {
    CHECK(n == -1);
    CHECK(error == EINVAL || error == ENOENT || error == ESRCH);
  }
}

static void rejected_command(const char *text)
{
  States_Control before[2];
  before[0] = containers[0]->core_cgroup->state;
  before[1] = containers[1]->core_cgroup->state;
  command(text, false);
  CHECK(containers[0]->core_cgroup->state == before[0]);
  CHECK(containers[1]->core_cgroup->state == before[1]);
  probe(1);
}

static rtems_task Init(rtems_task_argument arg)
{
  static const char *const invalid[] = {
    "not-a-command", "pause", "pause -1", "pause 4294967295",
    "resume 4294967295", "set 4294967295 10 100", "delete 4294967295"
  };
  RtemsContainerConfig config;
  rtems_status_code sc;
  rtems_event_set received;
  rtems_id deleted_id;
  uint32_t count;
  unsigned i;
  char cmd[80];

  (void) arg;
  abnormal_begin();
  manager_id = rtems_task_self();
  rtems_containerfs_register_cpuctl();
  control_fd = open("/cpuctl", O_WRONLY);
  rtems_test_assert(control_fd >= 0);
  rtems_unified_container_config_initialize(&config);
  config.flags = RTEMS_UNIFIED_CONTAINER_CPU;
  /* Avoid quota replenishment automatically resuming a manually paused task. */
  config.cgroup_config.cpu_quota = 1000000;
  config.cgroup_config.cpu_period = 1000000;
  config.cgroup_config.memory_limit = 0;
  config.cgroup_config.blkio_limit = 0;
  for (i = 0; i < 2; ++i) {
    rtems_test_assert(rtems_unified_container_create(
      &config, &containers[i]
    ) == RTEMS_SUCCESSFUL);
    sc = rtems_task_create(
      rtems_build_name('A', 'B', 'W', '0' + i), 9,
      2 * RTEMS_MINIMUM_STACK_SIZE, RTEMS_DEFAULT_MODES,
      RTEMS_DEFAULT_ATTRIBUTES, &worker_ids[i]
    );
    directive_failed(sc, "create lifecycle-test worker");
    rtems_test_assert(rtems_task_start(worker_ids[i], worker, i) == RTEMS_SUCCESSFUL);
    receive_event(READY(i));
    probe(i);
  }

  CHECK(rtems_unified_container_pause(NULL) == RTEMS_INVALID_ADDRESS);
  CHECK(rtems_unified_container_resume(NULL) == RTEMS_INVALID_ADDRESS);
  CHECK(rtems_cgroup_get_task_count(UINT32_MAX, &count) == RTEMS_INVALID_ID);
  for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
    rejected_command(invalid[i]);
    probe(0);
  }

  puts("[state] resume before pause must be rejected");
  CHECK(rtems_unified_container_resume(containers[0]) == RTEMS_INCORRECT_STATE);
  snprintf(cmd, sizeof(cmd), "resume %" PRIu32, containers[0]->cgroup_id);
  rejected_command(cmd);
  probe(0);

  snprintf(cmd, sizeof(cmd), "pause %" PRIu32, containers[0]->cgroup_id);
  command(cmd, true);
  CHECK((containers[0]->core_cgroup->state & STATES_WAITING_FOR_CGROUP_CPU_QUOTA) != 0);
  puts("[state] duplicate pause must be rejected and remain paused");
  CHECK(rtems_unified_container_pause(containers[0]) == RTEMS_INCORRECT_STATE);
  rejected_command(cmd);
  rtems_test_assert(rtems_event_send(worker_ids[0], PROBE) == RTEMS_SUCCESSFUL);
  sc = rtems_event_receive(
    ACK(0), RTEMS_EVENT_ALL | RTEMS_WAIT, 5, &received
  );
  /* If pause is ineffective, consume the response and continue.  The
   * contract failure is already recorded; do not turn it into a test hang. */
  CHECK(sc == RTEMS_TIMEOUT || sc == RTEMS_SUCCESSFUL);
  probe(1);

  snprintf(cmd, sizeof(cmd), "resume %" PRIu32, containers[0]->cgroup_id);
  command(cmd, true);
  /* The probe queued during pause must now complete. */
  if (sc == RTEMS_TIMEOUT) {
    receive_event(ACK(0));
  }
  probe(0);
  probe(1);
  CHECK((containers[0]->core_cgroup->state & STATES_WAITING_FOR_CGROUP_CPU_QUOTA) == 0);

  rtems_test_assert(rtems_event_send(worker_ids[0], STOP) == RTEMS_SUCCESSFUL);
  receive_event(DONE(0));
  deleted_id = containers[0]->cgroup_id;
  rtems_test_assert(rtems_unified_container_delete(containers[0]) == RTEMS_SUCCESSFUL);
  containers[0] = NULL;
  /* Use the saved numeric ID, never dereference a freed container pointer. */
  CHECK(rtems_cgroup_get_task_count(deleted_id, &count) == RTEMS_INVALID_ID);
  CHECK(rtems_cgroup_delete(deleted_id) == RTEMS_INVALID_ID);
  for (i = 0; i < 3; ++i) {
    static const char *const operations[] = {"pause", "resume", "delete"};
    States_Control before = containers[1]->core_cgroup->state;
    snprintf(cmd, sizeof(cmd), "%s %" PRIu32, operations[i], deleted_id);
    command(cmd, false);
    CHECK(containers[1]->core_cgroup->state == before);
    probe(1);
  }
  rtems_test_assert(rtems_event_send(worker_ids[1], STOP) == RTEMS_SUCCESSFUL);
  receive_event(DONE(1));
  rtems_test_assert(rtems_unified_container_delete(containers[1]) == RTEMS_SUCCESSFUL);
  rtems_test_assert(close(control_fd) == 0);
  abnormal_finish();
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_MAXIMUM_TASKS 4
/* confdefs budgets one minimum stack per task; these three tasks use two. */
#define CONFIGURE_EXTRA_TASK_STACKS (3 * RTEMS_MINIMUM_STACK_SIZE)
/* cgroup task membership nodes are allocated from the workspace at runtime. */
#define CONFIGURE_MEMORY_OVERHEAD 16
#define CONFIGURE_MAXIMUM_CGROUPS 2
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 8
#define CONFIGURE_INIT_TASK_PRIORITY 10
#define CONFIGURE_INIT_TASK_STACK_SIZE (4 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
