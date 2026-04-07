#ifndef _RTEMS_SCORE_CONTAINERLOG_H
#define _RTEMS_SCORE_CONTAINERLOG_H

#include <rtems.h>
#include <rtems/printer.h>
#include <rtems/rtems/status.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CONTAINER_LOG_ERROR = 0,
  CONTAINER_LOG_WARN = 1,
  CONTAINER_LOG_INFO = 2,
  CONTAINER_LOG_DEBUG = 3,
  CONTAINER_LOG_TRACE = 4
} container_log_level_t;

typedef enum {
  CONTAINER_LOG_TARGET_CONSOLE = 0x01,
  CONTAINER_LOG_TARGET_FILE = 0x02,
  CONTAINER_LOG_TARGET_MEMORY = 0x04
} container_log_target_t;

typedef struct {
  container_log_level_t min_level;
  container_log_target_t targets;
  char log_file_path[256];
  size_t max_file_size;
  uint32_t max_memory_entries;
  bool enable_timestamp;
  bool enable_thread_id;
  bool enable_function_name;
} container_log_config_t;

typedef struct {
  container_log_level_t level;
  uint32_t timestamp_sec;
  uint32_t timestamp_usec;
  rtems_id thread_id;
  char function_name[64];
  char message[512];
} container_log_entry_t;

typedef struct {
  container_log_entry_t *entries;
  uint32_t max_entries;
  uint32_t current_count;
  uint32_t write_index;
  rtems_id mutex_id;
} container_log_buffer_t;

typedef struct {
  container_log_config_t config;
  container_log_buffer_t buffer;
  FILE *log_file;
  bool initialized;
  rtems_id file_mutex_id;
} container_log_state_t;

container_log_config_t container_log_get_default_config(void);
rtems_status_code container_log_initialize(const container_log_config_t *config);
void container_log_destroy(void);
void container_log_insert(
  container_log_level_t level,
  const char *function_name,
  const char *format,
  ...
);
void container_log_insert_va(
  container_log_level_t level,
  const char *function_name,
  const char *format,
  va_list args
);
void container_log_set_level(container_log_level_t level);
container_log_level_t container_log_get_level(void);
void container_log_flush(void);
uint32_t container_log_get_entries(container_log_entry_t *entries, uint32_t max_entries);
void container_log_clear_buffer(void);
void container_log_print_stats(const rtems_printer *printer);
const char *container_log_level_to_string(container_log_level_t level);

#ifdef RTEMSCFG_CONTAINER_LOG
#define CONTAINER_LOG_INSERT_ERROR(fmt, ...) \
  container_log_insert(CONTAINER_LOG_ERROR, __func__, fmt, ##__VA_ARGS__)
#define CONTAINER_LOG_INSERT_WARN(fmt, ...) \
  container_log_insert(CONTAINER_LOG_WARN, __func__, fmt, ##__VA_ARGS__)
#define CONTAINER_LOG_INSERT_INFO(fmt, ...) \
  container_log_insert(CONTAINER_LOG_INFO, __func__, fmt, ##__VA_ARGS__)
#define CONTAINER_LOG_INSERT_DEBUG(fmt, ...) \
  container_log_insert(CONTAINER_LOG_DEBUG, __func__, fmt, ##__VA_ARGS__)
#define CONTAINER_LOG_INSERT_TRACE(fmt, ...) \
  container_log_insert(CONTAINER_LOG_TRACE, __func__, fmt, ##__VA_ARGS__)

#define CONTAINER_LOG_ERROR(fmt, ...) CONTAINER_LOG_INSERT_ERROR(fmt, ##__VA_ARGS__)
#define CONTAINER_LOG_WARN(fmt, ...) CONTAINER_LOG_INSERT_WARN(fmt, ##__VA_ARGS__)
#define CONTAINER_LOG_INFO(fmt, ...) CONTAINER_LOG_INSERT_INFO(fmt, ##__VA_ARGS__)
#define CONTAINER_LOG_DEBUG(fmt, ...) CONTAINER_LOG_INSERT_DEBUG(fmt, ##__VA_ARGS__)
#define CONTAINER_LOG_TRACE(fmt, ...) CONTAINER_LOG_INSERT_TRACE(fmt, ##__VA_ARGS__)
#else
#define CONTAINER_LOG_INSERT_ERROR(fmt, ...) do { } while (0)
#define CONTAINER_LOG_INSERT_WARN(fmt, ...) do { } while (0)
#define CONTAINER_LOG_INSERT_INFO(fmt, ...) do { } while (0)
#define CONTAINER_LOG_INSERT_DEBUG(fmt, ...) do { } while (0)
#define CONTAINER_LOG_INSERT_TRACE(fmt, ...) do { } while (0)

#define CONTAINER_LOG_ERROR(fmt, ...) do { } while (0)
#define CONTAINER_LOG_WARN(fmt, ...) do { } while (0)
#define CONTAINER_LOG_INFO(fmt, ...) do { } while (0)
#define CONTAINER_LOG_DEBUG(fmt, ...) do { } while (0)
#define CONTAINER_LOG_TRACE(fmt, ...) do { } while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif
