#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems/score/containerlog.h>

#include <rtems/rtems/clock.h>
#include <rtems/rtems/sem.h>
#include <rtems/rtems/tasks.h>

#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static container_log_state_t g_log_state;

#ifdef RTEMSCFG_CONTAINER_LOG
static const char *const g_log_level_strings[] = {
  "ERROR",
  "WARN ",
  "INFO ",
  "DEBUG",
  "TRACE"
};

static void container_log_format_message(
  char *buffer,
  size_t buffer_size,
  const container_log_entry_t *entry
)
{
  int len;

  len = 0;

  if (g_log_state.config.enable_timestamp) {
    struct tm tm_info;
    time_t seconds;

    seconds = (time_t) entry->timestamp_sec;
    localtime_r(&seconds, &tm_info);
    len += snprintf(
      buffer + len,
      buffer_size - (size_t) len,
      "[%04d-%02d-%02d %02d:%02d:%02d.%06" PRIu32 "] ",
      tm_info.tm_year + 1900,
      tm_info.tm_mon + 1,
      tm_info.tm_mday,
      tm_info.tm_hour,
      tm_info.tm_min,
      tm_info.tm_sec,
      entry->timestamp_usec
    );
  }

  len += snprintf(
    buffer + len,
    buffer_size - (size_t) len,
    "[%s] ",
    container_log_level_to_string(entry->level)
  );

  if (g_log_state.config.enable_thread_id) {
    len += snprintf(
      buffer + len,
      buffer_size - (size_t) len,
      "[TID:0x%08" PRIx32 "] ",
      entry->thread_id
    );
  }

  if (g_log_state.config.enable_function_name && entry->function_name[0] != '\0') {
    len += snprintf(
      buffer + len,
      buffer_size - (size_t) len,
      "[%s] ",
      entry->function_name
    );
  }

  (void) snprintf(buffer + len, buffer_size - (size_t) len, "%s", entry->message);
}

static void container_log_to_console(const container_log_entry_t *entry)
{
  char buffer[1024];

  container_log_format_message(buffer, sizeof(buffer), entry);
  fputs(buffer, stdout);
  fputc('\n', stdout);
}

static void container_log_to_file(const container_log_entry_t *entry)
{
  char buffer[1024];

  if (g_log_state.log_file == NULL || g_log_state.file_mutex_id == 0) {
    return;
  }

  if (rtems_semaphore_obtain(
        g_log_state.file_mutex_id,
        RTEMS_NO_WAIT,
        RTEMS_NO_TIMEOUT
      ) != RTEMS_SUCCESSFUL) {
    return;
  }

  container_log_format_message(buffer, sizeof(buffer), entry);
  fprintf(g_log_state.log_file, "%s\n", buffer);
  fflush(g_log_state.log_file);
  rtems_semaphore_release(g_log_state.file_mutex_id);
}

static void container_log_to_memory(const container_log_entry_t *entry)
{
  if (g_log_state.buffer.entries == NULL || g_log_state.buffer.mutex_id == 0) {
    return;
  }

  if (rtems_semaphore_obtain(
        g_log_state.buffer.mutex_id,
        RTEMS_NO_WAIT,
        RTEMS_NO_TIMEOUT
      ) != RTEMS_SUCCESSFUL) {
    return;
  }

  memcpy(
    &g_log_state.buffer.entries[g_log_state.buffer.write_index],
    entry,
    sizeof(*entry)
  );
  g_log_state.buffer.write_index =
    (g_log_state.buffer.write_index + 1U) % g_log_state.buffer.max_entries;

  if (g_log_state.buffer.current_count < g_log_state.buffer.max_entries) {
    ++g_log_state.buffer.current_count;
  }

  rtems_semaphore_release(g_log_state.buffer.mutex_id);
}

static rtems_status_code container_log_open_file(void)
{
  g_log_state.log_file = fopen(g_log_state.config.log_file_path, "a");
  if (g_log_state.log_file == NULL) {
    return RTEMS_IO_ERROR;
  }

  return RTEMS_SUCCESSFUL;
}

static void container_log_close_file(void)
{
  if (g_log_state.log_file != NULL) {
    fclose(g_log_state.log_file);
    g_log_state.log_file = NULL;
  }
}
#endif

container_log_config_t container_log_get_default_config(void)
{
  container_log_config_t config;

  memset(&config, 0, sizeof(config));
#ifdef RTEMSCFG_CONTAINER_LOG
  config.min_level = CONTAINER_LOG_INFO;
  config.targets =
    (container_log_target_t) (CONTAINER_LOG_TARGET_CONSOLE | CONTAINER_LOG_TARGET_MEMORY);
  strncpy(config.log_file_path, "/tmp/container.log", sizeof(config.log_file_path) - 1);
  config.max_file_size = 1024U * 1024U;
  config.max_memory_entries = 256U;
  config.enable_timestamp = true;
  config.enable_thread_id = true;
  config.enable_function_name = true;
#endif
  return config;
}

rtems_status_code container_log_initialize(const container_log_config_t *config)
{
#ifdef RTEMSCFG_CONTAINER_LOG
  rtems_status_code sc;

  if (g_log_state.initialized) {
    return RTEMS_RESOURCE_IN_USE;
  }

  if (config == NULL) {
    return RTEMS_INVALID_ADDRESS;
  }

  memset(&g_log_state, 0, sizeof(g_log_state));
  memcpy(&g_log_state.config, config, sizeof(*config));

  if ((g_log_state.config.targets & CONTAINER_LOG_TARGET_MEMORY) != 0U) {
    g_log_state.buffer.entries = calloc(
      g_log_state.config.max_memory_entries,
      sizeof(container_log_entry_t)
    );
    if (g_log_state.buffer.entries == NULL) {
      return RTEMS_NO_MEMORY;
    }

    g_log_state.buffer.max_entries = g_log_state.config.max_memory_entries;

    sc = rtems_semaphore_create(
      rtems_build_name('L', 'O', 'G', 'M'),
      1,
      RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY | RTEMS_BINARY_SEMAPHORE,
      0,
      &g_log_state.buffer.mutex_id
    );
    if (sc != RTEMS_SUCCESSFUL) {
      free(g_log_state.buffer.entries);
      memset(&g_log_state, 0, sizeof(g_log_state));
      return sc;
    }
  }

  if ((g_log_state.config.targets & CONTAINER_LOG_TARGET_FILE) != 0U) {
    sc = container_log_open_file();
    if (sc != RTEMS_SUCCESSFUL) {
      container_log_destroy();
      return sc;
    }

    sc = rtems_semaphore_create(
      rtems_build_name('L', 'O', 'G', 'F'),
      1,
      RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY | RTEMS_BINARY_SEMAPHORE,
      0,
      &g_log_state.file_mutex_id
    );
    if (sc != RTEMS_SUCCESSFUL) {
      container_log_destroy();
      return sc;
    }
  }

  g_log_state.initialized = true;
  CONTAINER_LOG_INFO("Container log subsystem initialized");
  return RTEMS_SUCCESSFUL;
#else
  (void) config;
  return RTEMS_NOT_CONFIGURED;
#endif
}

void container_log_destroy(void)
{
#ifdef RTEMSCFG_CONTAINER_LOG
  if (!g_log_state.initialized) {
    return;
  }

  CONTAINER_LOG_INFO("Container log subsystem destroying");
  container_log_flush();

  if (g_log_state.file_mutex_id != 0) {
    rtems_semaphore_delete(g_log_state.file_mutex_id);
    g_log_state.file_mutex_id = 0;
  }

  if (g_log_state.buffer.mutex_id != 0) {
    rtems_semaphore_delete(g_log_state.buffer.mutex_id);
    g_log_state.buffer.mutex_id = 0;
  }

  container_log_close_file();
  free(g_log_state.buffer.entries);
  memset(&g_log_state, 0, sizeof(g_log_state));
#endif
}

void container_log_insert(
  container_log_level_t level,
  const char *function_name,
  const char *format,
  ...
)
{
  va_list args;

  va_start(args, format);
  container_log_insert_va(level, function_name, format, args);
  va_end(args);
}

void container_log_insert_va(
  container_log_level_t level,
  const char *function_name,
  const char *format,
  va_list args
)
{
#ifdef RTEMSCFG_CONTAINER_LOG
  container_log_entry_t entry;
  struct timespec now;

  if (!g_log_state.initialized || level > g_log_state.config.min_level) {
    return;
  }

  memset(&entry, 0, sizeof(entry));
  entry.level = level;

  if (g_log_state.config.enable_timestamp) {
    clock_gettime(CLOCK_REALTIME, &now);
    entry.timestamp_sec = (uint32_t) now.tv_sec;
    entry.timestamp_usec = (uint32_t) (now.tv_nsec / 1000L);
  }

  if (g_log_state.config.enable_thread_id) {
    entry.thread_id = rtems_task_self();
  }

  if (g_log_state.config.enable_function_name && function_name != NULL) {
    strncpy(entry.function_name, function_name, sizeof(entry.function_name) - 1);
  }

  vsnprintf(entry.message, sizeof(entry.message), format, args);

  if ((g_log_state.config.targets & CONTAINER_LOG_TARGET_CONSOLE) != 0U) {
    container_log_to_console(&entry);
  }

  if ((g_log_state.config.targets & CONTAINER_LOG_TARGET_FILE) != 0U) {
    container_log_to_file(&entry);
  }

  if ((g_log_state.config.targets & CONTAINER_LOG_TARGET_MEMORY) != 0U) {
    container_log_to_memory(&entry);
  }
#else
  (void) level;
  (void) function_name;
  (void) format;
  (void) args;
#endif
}

void container_log_set_level(container_log_level_t level)
{
#ifdef RTEMSCFG_CONTAINER_LOG
  if (g_log_state.initialized) {
    g_log_state.config.min_level = level;
  }
#else
  (void) level;
#endif
}

container_log_level_t container_log_get_level(void)
{
#ifdef RTEMSCFG_CONTAINER_LOG
  if (g_log_state.initialized) {
    return g_log_state.config.min_level;
  }
#endif
  return CONTAINER_LOG_ERROR;
}

void container_log_flush(void)
{
#ifdef RTEMSCFG_CONTAINER_LOG
  if (g_log_state.log_file != NULL) {
    fflush(g_log_state.log_file);
  }
#endif
}

uint32_t container_log_get_entries(container_log_entry_t *entries, uint32_t max_entries)
{
#ifdef RTEMSCFG_CONTAINER_LOG
  uint32_t count;
  uint32_t start;
  uint32_t i;

  if (
    !g_log_state.initialized ||
    entries == NULL ||
    max_entries == 0U ||
    g_log_state.buffer.entries == NULL ||
    g_log_state.buffer.mutex_id == 0
  ) {
    return 0;
  }

  if (rtems_semaphore_obtain(
        g_log_state.buffer.mutex_id,
        RTEMS_NO_WAIT,
        RTEMS_NO_TIMEOUT
      ) != RTEMS_SUCCESSFUL) {
    return 0;
  }

  count = g_log_state.buffer.current_count;
  if (count > max_entries) {
    count = max_entries;
  }

  start =
    (g_log_state.buffer.write_index + g_log_state.buffer.max_entries - count) %
    g_log_state.buffer.max_entries;

  for (i = 0; i < count; ++i) {
    memcpy(
      &entries[i],
      &g_log_state.buffer.entries[(start + i) % g_log_state.buffer.max_entries],
      sizeof(entries[i])
    );
  }

  rtems_semaphore_release(g_log_state.buffer.mutex_id);
  return count;
#else
  (void) entries;
  (void) max_entries;
  return 0;
#endif
}

void container_log_clear_buffer(void)
{
#ifdef RTEMSCFG_CONTAINER_LOG
  if (
    !g_log_state.initialized ||
    g_log_state.buffer.entries == NULL ||
    g_log_state.buffer.mutex_id == 0
  ) {
    return;
  }

  if (rtems_semaphore_obtain(
        g_log_state.buffer.mutex_id,
        RTEMS_NO_WAIT,
        RTEMS_NO_TIMEOUT
      ) != RTEMS_SUCCESSFUL) {
    return;
  }

  memset(
    g_log_state.buffer.entries,
    0,
    sizeof(container_log_entry_t) * g_log_state.buffer.max_entries
  );
  g_log_state.buffer.current_count = 0;
  g_log_state.buffer.write_index = 0;
  rtems_semaphore_release(g_log_state.buffer.mutex_id);
#endif
}

void container_log_print_stats(const rtems_printer *printer)
{
  (void) printer;

#ifdef RTEMSCFG_CONTAINER_LOG
  if (!g_log_state.initialized) {
    printf("Container log subsystem not initialized\n");
    return;
  }

  printf("\n=== Container Log Statistics ===\n");
  printf("Initialized: %s\n", g_log_state.initialized ? "Yes" : "No");
  printf("Min Level: %s\n", container_log_level_to_string(g_log_state.config.min_level));
  printf("Targets:");
  if ((g_log_state.config.targets & CONTAINER_LOG_TARGET_CONSOLE) != 0U) {
    printf(" console");
  }
  if ((g_log_state.config.targets & CONTAINER_LOG_TARGET_FILE) != 0U) {
    printf(" file");
  }
  if ((g_log_state.config.targets & CONTAINER_LOG_TARGET_MEMORY) != 0U) {
    printf(" memory");
  }
  printf("\n");
  printf(
    "Buffered Entries: %" PRIu32 "/%" PRIu32 "\n",
    g_log_state.buffer.current_count,
    g_log_state.buffer.max_entries
  );
  printf("Log File: %s\n", g_log_state.config.log_file_path);
#else
  printf("Container log subsystem is disabled\n");
#endif
}

const char *container_log_level_to_string(container_log_level_t level)
{
#ifdef RTEMSCFG_CONTAINER_LOG
  if ((size_t) level < (sizeof(g_log_level_strings) / sizeof(g_log_level_strings[0]))) {
    return g_log_level_strings[level];
  }
#else
  (void) level;
#endif
  return "UNKNOWN";
}
