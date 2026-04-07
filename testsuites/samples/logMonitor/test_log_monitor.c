/*
 * Basic log/monitor migration test for the RTEMS container branch.
 */

#include <rtems.h>
#include <rtems/printer.h>
#include <rtems/score/container.h>
#include <rtems/score/containerlog.h>
#include <rtems/score/monitor.h>
#include <rtems/score/threadimpl.h>
#include <rtems/score/utsContainer.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

static void print_recent_entries(void)
{
  container_log_entry_t entries[8];
  uint32_t count;
  uint32_t i;

  count = container_log_get_entries(entries, 8);
  printf("\nRecent container log entries: %" PRIu32 "\n", count);
  for (i = 0; i < count; ++i) {
    printf(
      "  [%s] %s\n",
      container_log_level_to_string(entries[i].level),
      entries[i].message
    );
  }
}

static rtems_task Init(rtems_task_argument arg)
{
  container_log_config_t config;
  Container *root;
  Thread_Control *self;
  RtemsMonitor snapshot;
  rtems_printer printer;

  (void) arg;

  printf("*** BEGIN LOG_MONITOR TEST ***\n");

  config = container_log_get_default_config();
  config.min_level = CONTAINER_LOG_TRACE;
  config.targets =
    (container_log_target_t) (CONTAINER_LOG_TARGET_CONSOLE | CONTAINER_LOG_TARGET_MEMORY);

  if (container_log_initialize(&config) != RTEMS_SUCCESSFUL) {
    printf("container_log_initialize failed\n");
  }

  root = rtems_container_get_root();
  self = (Thread_Control *) _Thread_Get_executing();
  if (root == NULL || self == NULL || self->container == NULL) {
    printf("container framework is not available\n");
    container_log_destroy();
    exit(1);
  }

  rtems_monitor_initialize();
  rtems_monitor_sample();
  rtems_monitor_get_snapshot(&snapshot);
  rtems_monitor_print_line();

  printf("Monitor snapshot:\n");
#ifdef RTEMSCFG_MONITOR_CPU
  printf(
    "  CPU uptime: %" PRIu32 ".%06" PRIu32 "s\n",
    snapshot.cpu.uptime_seconds,
    snapshot.cpu.uptime_microseconds
  );
#endif
#ifdef RTEMSCFG_MONITOR_MEM
  printf(
    "  MEM used/free: %" PRIu64 "/%" PRIu64 "\n",
    snapshot.mem.bytes_used,
    snapshot.mem.bytes_free
  );
#endif
#ifdef RTEMSCFG_MONITOR_NET
  printf(
    "  NET rx/tx bytes: %" PRIu64 "/%" PRIu64 "\n",
    snapshot.net.rx_bytes,
    snapshot.net.tx_bytes
  );
#endif

#ifdef RTEMSCFG_UTS_CONTAINER
  if (root->utsContainer != NULL) {
    UtsContainer *child;
    UtsContainer *root_uts;

    root_uts = root->utsContainer;
    child = rtems_uts_container_create("log-monitor");
    if (child != NULL) {
      rtems_uts_container_move_task(root_uts, child, self);
      child->rc++;
      rtems_uts_container_move_task(child, root_uts, self);
      child->rc--;
      rtems_uts_container_delete(child);
    }
  }
#endif

  rtems_print_printer_printf(&printer);
  rtems_monitor_print_detailed_report(&printer);
  container_log_print_stats(&printer);
  print_recent_entries();
  container_log_destroy();

  printf("*** END LOG_MONITOR TEST ***\n");
  exit(0);
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_MAXIMUM_TASKS 4
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
