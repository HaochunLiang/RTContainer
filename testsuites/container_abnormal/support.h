/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef CONTAINER_ABNORMAL_SUPPORT_H
#define CONTAINER_ABNORMAL_SUPPORT_H

#include <rtems.h>
#include <tmacros.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static unsigned abnormal_failures;
static rtems_id abnormal_watchdog_id;

/* Record contract failures, so recovery and cleanup still get exercised. */
#define CHECK(condition) do { \
  if (!(condition)) { \
    printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, #condition); \
    ++abnormal_failures; \
  } \
} while (0)

static rtems_task abnormal_watchdog(rtems_task_argument arg)
{
  (void) arg;
  rtems_task_wake_after(30 * rtems_clock_get_ticks_per_second());
  puts("[FAIL] test exceeded 30 seconds (possible deadlock)");
  rtems_test_assert(false);
}

static void abnormal_begin(void)
{
  rtems_status_code sc;

  TEST_BEGIN();
  sc = rtems_task_create(
    rtems_build_name('A', 'B', 'W', 'D'), 1,
    2 * RTEMS_MINIMUM_STACK_SIZE, RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES, &abnormal_watchdog_id
  );
  directive_failed(sc, "create abnormal-test watchdog");
  rtems_test_assert(rtems_task_start(
    abnormal_watchdog_id, abnormal_watchdog, 0
  ) == RTEMS_SUCCESSFUL);
}

static void abnormal_finish(void)
{
  rtems_test_assert(rtems_task_delete(abnormal_watchdog_id) == RTEMS_SUCCESSFUL);
  printf("[result] contract failures: %u\n", abnormal_failures);
  /* A failing contract must never produce the successful END marker. */
  rtems_test_assert(abnormal_failures == 0);
  TEST_END();
  rtems_test_exit(0);
}

#endif
