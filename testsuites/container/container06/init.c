#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems.h>
#include <rtems/counter.h>
#include <rtems/score/container.h>
#include <rtems/score/threadimpl.h>
#include <tmacros.h>

#include <inttypes.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>

const char rtems_test_name[] = "CONTAINER 06";

#define WARMUP_SWITCHES 100u
#define MEASURED_SWITCHES 1000u

#define PING_EVENT RTEMS_EVENT_0
#define PONG_EVENT RTEMS_EVENT_0
#define RECEIVER_READY_EVENT RTEMS_EVENT_0
#define SENDER_DONE_EVENT RTEMS_EVENT_1
#define RECEIVER_DONE_EVENT RTEMS_EVENT_2

typedef struct {
  rtems_counter_ticks total_ticks;
  rtems_counter_ticks min_ticks;
  rtems_counter_ticks max_ticks;
} SwitchStats;

static rtems_id init_task_id;
static rtems_id sender_task_id;
static rtems_id receiver_task_id;
static RtemsContainer *sender_container;
static RtemsContainer *receiver_container;
static volatile rtems_counter_ticks switch_start_tick;
static SwitchStats current_stats;

static void pin_task_to_cpu_0(rtems_id task_id)
{
#if defined(RTEMS_SMP)
  cpu_set_t set;
  rtems_status_code sc;

  CPU_ZERO(&set);
  CPU_SET(0, &set);
  sc = rtems_task_set_affinity(task_id, sizeof(set), &set);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
#else
  (void) task_id;
#endif
}

static void enter_container(RtemsContainer *container)
{
  Thread_Control *self;
  rtems_status_code sc;

  if (container == NULL) {
    return;
  }

  self = _Thread_Get_executing();
  sc = rtems_unified_container_enter(container, self);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert(
    self->cgroup == rtems_unified_container_get_core_cgroup(container)
  );
}

static void leave_container(RtemsContainer *container)
{
  rtems_status_code sc;

  if (container == NULL) {
    return;
  }

  sc = rtems_unified_container_leave(container, _Thread_Get_executing());
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
}

static rtems_task receiver_task(rtems_task_argument arg)
{
  rtems_event_set received;
  rtems_status_code sc;
  uint32_t i;

  (void) arg;

  enter_container(receiver_container);

  sc = rtems_event_send(init_task_id, RECEIVER_READY_EVENT);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  for (i = 0; i < WARMUP_SWITCHES + MEASURED_SWITCHES; ++i) {
    rtems_counter_ticks elapsed;

    sc = rtems_event_receive(
      PING_EVENT,
      RTEMS_EVENT_ALL | RTEMS_WAIT,
      RTEMS_NO_TIMEOUT,
      &received
    );
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    elapsed = rtems_counter_difference(
      rtems_counter_read(),
      switch_start_tick
    );

    if (i >= WARMUP_SWITCHES) {
      current_stats.total_ticks += elapsed;
      if (elapsed < current_stats.min_ticks) {
        current_stats.min_ticks = elapsed;
      }
      if (elapsed > current_stats.max_ticks) {
        current_stats.max_ticks = elapsed;
      }
    }

    sc = rtems_event_send(sender_task_id, PONG_EVENT);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  }

  leave_container(receiver_container);

  sc = rtems_event_send(init_task_id, RECEIVER_DONE_EVENT);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_task_exit();
}

static rtems_task sender_task(rtems_task_argument arg)
{
  rtems_event_set received;
  rtems_status_code sc;
  uint32_t i;

  (void) arg;

  enter_container(sender_container);

  for (i = 0; i < WARMUP_SWITCHES + MEASURED_SWITCHES; ++i) {
    switch_start_tick = rtems_counter_read();

    sc = rtems_event_send(receiver_task_id, PING_EVENT);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    sc = rtems_event_receive(
      PONG_EVENT,
      RTEMS_EVENT_ALL | RTEMS_WAIT,
      RTEMS_NO_TIMEOUT,
      &received
    );
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  }

  leave_container(sender_container);

  sc = rtems_event_send(init_task_id, SENDER_DONE_EVENT);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_task_exit();
}

static void run_switch_phase(
  RtemsContainer *phase_sender_container,
  RtemsContainer *phase_receiver_container,
  SwitchStats *stats
)
{
  rtems_event_set received;
  rtems_status_code sc;

  sender_container = phase_sender_container;
  receiver_container = phase_receiver_container;
  current_stats.total_ticks = 0;
  current_stats.min_ticks = (rtems_counter_ticks) -1;
  current_stats.max_ticks = 0;
  switch_start_tick = 0;

  sc = rtems_task_create(
    rtems_build_name('R', 'X', '0', '6'),
    20,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &receiver_task_id
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_task_create(
    rtems_build_name('T', 'X', '0', '6'),
    20,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &sender_task_id
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  pin_task_to_cpu_0(receiver_task_id);
  pin_task_to_cpu_0(sender_task_id);

  sc = rtems_task_start(receiver_task_id, receiver_task, 0);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_event_receive(
    RECEIVER_READY_EVENT,
    RTEMS_EVENT_ALL | RTEMS_WAIT,
    RTEMS_NO_TIMEOUT,
    &received
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_task_start(sender_task_id, sender_task, 0);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_event_receive(
    SENDER_DONE_EVENT | RECEIVER_DONE_EVENT,
    RTEMS_EVENT_ALL | RTEMS_WAIT,
    RTEMS_NO_TIMEOUT,
    &received
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert(current_stats.min_ticks != (rtems_counter_ticks) -1);

  *stats = current_stats;
}

static uint64_t average_nanoseconds(const SwitchStats *stats)
{
  return rtems_counter_ticks_to_nanoseconds(stats->total_ticks) /
    MEASURED_SWITCHES;
}

static void print_stats(const char *label, const SwitchStats *stats)
{
  printf(
    "[result] %s: samples=%" PRIu32
    ", avg=%" PRIu64 " ns, min=%" PRIu64 " ns, max=%" PRIu64 " ns\n",
    label,
    MEASURED_SWITCHES,
    average_nanoseconds(stats),
    rtems_counter_ticks_to_nanoseconds(stats->min_ticks),
    rtems_counter_ticks_to_nanoseconds(stats->max_ticks)
  );
}

static rtems_task Init(rtems_task_argument arg)
{
  RtemsContainerConfig config;
  RtemsContainer *container_a;
  RtemsContainer *container_b;
  SwitchStats host_stats;
  SwitchStats container_stats;
  rtems_status_code sc;
  uint64_t host_avg_ns;
  uint64_t container_avg_ns;
  int64_t overhead_ns;

  (void) arg;

  TEST_BEGIN();
  printf(
    "[init] measure host task switching and cross-container switching\n"
  );
  printf(
    "[init] warmup=%" PRIu32 ", measured samples=%" PRIu32 "\n",
    WARMUP_SWITCHES,
    MEASURED_SWITCHES
  );

  init_task_id = rtems_task_self();

  printf("[phase 1] host-to-host task switching\n");
  run_switch_phase(NULL, NULL, &host_stats);

  rtems_unified_container_config_initialize(&config);
  config.flags = RTEMS_UNIFIED_CONTAINER_CPU;
  config.cgroup_config.cpu_quota = 1000000u;
  config.cgroup_config.cpu_period = 1000000u;

  sc = rtems_unified_container_create(&config, &container_a);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  sc = rtems_unified_container_create(&config, &container_b);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert(
    rtems_unified_container_get_core_cgroup(container_a) !=
      rtems_unified_container_get_core_cgroup(container_b)
  );

  printf("[phase 2] container-A-to-container-B task switching\n");
  run_switch_phase(container_a, container_b, &container_stats);

  host_avg_ns = average_nanoseconds(&host_stats);
  container_avg_ns = average_nanoseconds(&container_stats);
  if (container_avg_ns >= host_avg_ns) {
    overhead_ns = (int64_t) (container_avg_ns - host_avg_ns);
  } else {
    overhead_ns = -(int64_t) (host_avg_ns - container_avg_ns);
  }

  print_stats("host task switch", &host_stats);
  print_stats("cross-container switch", &container_stats);
  printf(
    "[result] container switch overhead (container avg - host avg): "
    "%" PRId64 " ns\n",
    overhead_ns
  );

  sc = rtems_unified_container_delete(container_a);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  sc = rtems_unified_container_delete(container_b);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  TEST_END();
  rtems_test_exit(0);
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER

#define CONFIGURE_MAXIMUM_TASKS 4
#define CONFIGURE_MAXIMUM_CGROUPS 2

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_INIT
#include <rtems/confdefs.h>
