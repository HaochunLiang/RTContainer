/* SPDX-License-Identifier: BSD-2-Clause */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../support.h"
#include <rtems/imfs.h>
#include <rtems/libio.h>
#include <rtems/score/containerlog.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const char rtems_test_name[] = "CONTAINER ABNORMAL 05";

#define CAPACITY 8u
#define BURST_ENTRIES 257u
#define SINK_PATH "/abnormal-log-sink"

static container_log_entry_t entries[CAPACITY];
static bool sink_unavailable;
static unsigned sink_errors;
static char sink_data[8192];
static size_t sink_used;

/* This is an actual failing filesystem write target, not a mocked logger. */
static ssize_t sink_write(rtems_libio_t *iop, const void *buffer, size_t count)
{
  (void) iop;
  if (sink_unavailable) {
    ++sink_errors;
    errno = EIO;
    return -1;
  }
  if (count >= sizeof(sink_data) - sink_used) {
    errno = ENOSPC;
    return -1;
  }
  memcpy(sink_data + sink_used, buffer, count);
  sink_used += count;
  sink_data[sink_used] = '\0';
  return (ssize_t) count;
}

static const rtems_filesystem_file_handlers_r sink_handlers = {
  .open_h = rtems_filesystem_default_open,
  .close_h = rtems_filesystem_default_close,
  .read_h = rtems_filesystem_default_read,
  .write_h = sink_write,
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

static const IMFS_node_control sink_control = {
  .handlers = &sink_handlers,
  .node_initialize = IMFS_node_initialize_generic,
  .node_remove = IMFS_node_remove_default,
  .node_destroy = IMFS_node_destroy_default
};

static bool memory_contains(const char *message)
{
  uint32_t i;
  uint32_t count = container_log_get_entries(entries, CAPACITY);
  for (i = 0; i < count; ++i) {
    if (strcmp(entries[i].message, message) == 0) {
      return true;
    }
  }
  return false;
}

static void check_overflow(container_log_config_t *config)
{
  uint32_t i;
  uint32_t count;
  uint32_t overwritten;
  char expected[32];

  config->targets = CONTAINER_LOG_TARGET_MEMORY;
  rtems_test_assert(container_log_initialize(config) == RTEMS_SUCCESSFUL);
  /* Exclude the logger's own initialization entry from accounting. */
  container_log_clear_buffer();
  CHECK(container_log_get_entries(entries, CAPACITY) == 0);
  for (i = 0; i < BURST_ENTRIES; ++i) {
    container_log_insert(CONTAINER_LOG_INFO, __func__, "sequence=%" PRIu32, i);
  }
  count = container_log_get_entries(entries, CAPACITY);
  CHECK(count == CAPACITY);
  for (i = 0; i < count; ++i) {
    snprintf(expected, sizeof(expected), "sequence=%" PRIu32,
      BURST_ENTRIES - CAPACITY + i);
    CHECK(strcmp(entries[i].message, expected) == 0);
    CHECK(entries[i].level == CONTAINER_LOG_INFO);
  }
  /* Exact retained sequence range establishes how many old entries were lost. */
  overwritten = BURST_ENTRIES - count;
  CHECK(overwritten == 249);
  printf("[overflow] submitted=%u retained=%" PRIu32
    " overwritten=%" PRIu32 " (derived from retained entries)\n",
    BURST_ENTRIES, count, overwritten);
  puts("[coverage] logger has no public dropped-entry counter; sequence accounting checked");

  container_log_insert(CONTAINER_LOG_INFO, __func__, "after-overflow");
  CHECK(container_log_get_entries(entries, CAPACITY) == CAPACITY);
  CHECK(strcmp(entries[CAPACITY - 1].message, "after-overflow") == 0);
  container_log_clear_buffer();
  CHECK(container_log_get_entries(entries, CAPACITY) == 0);
  container_log_insert(CONTAINER_LOG_INFO, __func__, "after-clear");
  CHECK(container_log_get_entries(entries, CAPACITY) == 1);
  CHECK(strcmp(entries[0].message, "after-clear") == 0);
  container_log_destroy();
}

static void check_output_failure(container_log_config_t *config)
{
  size_t before_failure;
  unsigned errors_before_recovery;

  rtems_test_assert(IMFS_make_generic_node(
    SINK_PATH, S_IFCHR | S_IRUSR | S_IWUSR, &sink_control, NULL
  ) == 0);
  config->targets = (container_log_target_t) (
    CONTAINER_LOG_TARGET_FILE | CONTAINER_LOG_TARGET_MEMORY |
    CONTAINER_LOG_TARGET_CONSOLE
  );
  strcpy(config->log_file_path, SINK_PATH);
  rtems_test_assert(container_log_initialize(config) == RTEMS_SUCCESSFUL);
  container_log_clear_buffer();

  container_log_insert(CONTAINER_LOG_INFO, __func__, "file-before-fault");
  container_log_flush();
  CHECK(strstr(sink_data, "file-before-fault") != NULL);
  CHECK(memory_contains("file-before-fault"));
  before_failure = sink_used;

  sink_unavailable = true;
  container_log_insert(CONTAINER_LOG_ERROR, __func__, "file-during-fault");
  container_log_flush();
  CHECK(sink_errors > 0);
  CHECK(sink_used == before_failure);
  CHECK(strstr(sink_data, "file-during-fault") == NULL);
  CHECK(memory_contains("file-during-fault"));
  container_log_insert(CONTAINER_LOG_WARN, __func__, "memory-still-working");
  CHECK(memory_contains("memory-still-working"));
  CHECK(sink_used == before_failure);
  printf("[output] failing writes=%u, memory target continues\n", sink_errors);

  errors_before_recovery = sink_errors;
  sink_unavailable = false;
  /* Recover the same open destination without destroying/reinitializing logging. */
  container_log_insert(CONTAINER_LOG_INFO, __func__, "file-after-recovery");
  container_log_flush();
  CHECK(sink_errors == errors_before_recovery);
  CHECK(strstr(sink_data, "file-after-recovery") != NULL);
  CHECK(memory_contains("file-after-recovery"));
  container_log_destroy();
  CHECK(container_log_get_entries(entries, CAPACITY) == 0);
  rtems_test_assert(unlink(SINK_PATH) == 0);
}

static rtems_task Init(rtems_task_argument arg)
{
  container_log_config_t config;
  (void) arg;
  abnormal_begin();
  config = container_log_get_default_config();
  config.min_level = CONTAINER_LOG_INFO;
  config.max_memory_entries = CAPACITY;
  config.enable_timestamp = false;
  config.enable_thread_id = false;
  config.enable_function_name = false;
  check_overflow(&config);
  check_output_failure(&config);
  abnormal_finish();
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_MAXIMUM_TASKS 2
#define CONFIGURE_MAXIMUM_SEMAPHORES 4
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 8
#define CONFIGURE_INIT_TASK_PRIORITY 10
#define CONFIGURE_INIT_TASK_STACK_SIZE (8 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
