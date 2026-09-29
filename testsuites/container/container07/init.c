/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (C) 2026 RTEMS Container Project */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems.h>
#include <rtems/counter.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/score/container.h>
#include <rtems/score/threadimpl.h>
#include <tmacros.h>

#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "benchmark.h"

const char rtems_test_name[] = "CONTAINER 07";

#define READY_A RTEMS_EVENT_0
#define READY_B RTEMS_EVENT_1
#define DONE_A RTEMS_EVENT_2
#define DONE_B RTEMS_EVENT_3
#define GO RTEMS_EVENT_4
#define PING RTEMS_EVENT_5
#define PONG RTEMS_EVENT_6
#define FINISH RTEMS_EVENT_7

typedef struct {
  uint64_t begin;
  uint64_t ready;
  uint64_t api_return;
} StartupSample;

typedef struct {
  uint64_t deadline;
  uint64_t actual;
} TimerSample;

typedef struct {
  uint64_t sent;
  uint64_t peer_received;
  uint64_t peer_replied;
  uint64_t returned;
} SwitchSample;

typedef struct {
  rtems_id task;
  RtemsContainer *container;
  uint32_t round;
} Worker;

typedef enum {
  RUN_READY,
  START_READY,
  RUN_API,
  START_API,
  TIMER_WAKEUP,
  SWITCH_A_TO_B,
  SWITCH_B_TO_A,
  SWITCH_RTT,
  SWITCH_HALF_RTT,
  METRIC_COUNT
} Metric;

static const char *const metric_names[METRIC_COUNT] = {
  "run_ready", "start_ready", "run_api_return", "start_api_return",
  "timer_wakeup", "switch_a_to_b", "switch_b_to_a", "switch_rtt",
  "switch_half_rtt"
};

/* Keep all raw data until every timed phase is finished.  No timed printf. */
static StartupSample startup_samples[C07_ROUNDS][2][C07_STARTUP_SAMPLES];
static TimerSample timer_samples[C07_ROUNDS][C07_TIMER_SAMPLES];
static SwitchSample switch_samples[C07_ROUNDS][C07_SWITCH_SAMPLES];
static uint64_t sort_buffer[C07_SORT_CAPACITY];
static uint64_t timer_overruns[C07_ROUNDS];
static uint64_t timer_max_missed_periods[C07_ROUNDS];
static rtems_id init_task;
static Worker worker_a;
static Worker worker_b;

/* Access is serialized by Classic events, same CPU, same FIFO priority. */
static volatile uint64_t startup_ready;
static volatile uint64_t peer_received;
static volatile uint64_t peer_replied;
static volatile uint32_t request_sequence;
static volatile uint32_t reply_sequence;

static uint64_t now_ns(void)
{
  struct timespec now;
  int eno = clock_gettime(CLOCK_MONOTONIC, &now);

  rtems_test_assert(eno == 0);
  rtems_test_assert(now.tv_sec >= 0);
  return (uint64_t) now.tv_sec * C07_NS_PER_SECOND +
    (uint64_t) now.tv_nsec;
}

static rtems_interval timeout_ticks(uint64_t ns)
{
  uint64_t tick_ns = (uint64_t) C07_TICK_US * 1000u;
  uint64_t ticks = (ns + tick_ns - 1u) / tick_ns;

  rtems_test_assert(ticks > 0 && ticks <= UINT32_MAX);
  return (rtems_interval) ticks;
}

static void receive_events(rtems_event_set events, rtems_interval timeout)
{
  rtems_event_set received;
  rtems_status_code sc = rtems_event_receive(
    events, RTEMS_EVENT_ALL | RTEMS_WAIT, timeout, &received
  );

  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert(received == events);
}

static void send_events(rtems_id task, rtems_event_set events)
{
  rtems_status_code sc = rtems_event_send(task, events);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
}

static rtems_interval normal_timeout(void)
{
  return timeout_ticks((uint64_t) C07_WAIT_SECONDS * C07_NS_PER_SECOND);
}

static void pin_to_cpu_zero(rtems_id task)
{
#if defined(RTEMS_SMP)
  cpu_set_t set;
  cpu_set_t actual;
  rtems_status_code sc;

  CPU_ZERO(&set);
  CPU_SET(0, &set);
  sc = rtems_task_set_affinity(task, sizeof(set), &set);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  sc = rtems_task_get_affinity(task, sizeof(actual), &actual);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert(CPU_EQUAL(&set, &actual));
#else
  (void) task;
#endif
}

static void configure_container(RtemsContainerConfig *config)
{
  rtems_unified_container_config_initialize(config);
  config->flags = RTEMS_UNIFIED_CONTAINER_ALL;
  config->uts_name = "container07";
  /* Long equal budget/period: no intended CPU throttling during a phase. */
  config->cgroup_config.cpu_quota = C07_CPU_BUDGET_TICKS;
  config->cgroup_config.cpu_period = C07_CPU_BUDGET_TICKS;
  config->cgroup_config.memory_limit = 16u * 1024u * 1024u;
  config->cgroup_config.blkio_limit = 1024u * 1024u * 1024u;
  config->io_system_read_bps = 1024u * 1024u * 1024u;
  config->io_system_write_bps = 1024u * 1024u * 1024u;
  config->io_read_bps_limit = 1024u * 1024u * 1024u;
  config->io_write_bps_limit = 1024u * 1024u * 1024u;
}

static void create_container(Worker *worker)
{
  RtemsContainerConfig config;
  rtems_status_code sc;

  configure_container(&config);
  sc = rtems_unified_container_create(&config, &worker->container);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert(worker->container != NULL);
}

static void create_task(Worker *worker, rtems_name name)
{
  rtems_status_code sc = rtems_task_create(
    name, C07_WORKER_PRIORITY, 4u * RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_PREEMPT | RTEMS_NO_TIMESLICE | RTEMS_NO_ASR,
    RTEMS_DEFAULT_ATTRIBUTES, &worker->task
  );

  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  pin_to_cpu_zero(worker->task);
}

static rtems_status_code attach_self(Worker *worker)
{
  return rtems_unified_container_enter(
    worker->container, _Thread_Get_executing()
  );
}

static void verify_worker(Worker *worker)
{
  Thread_Control *self = _Thread_Get_executing();
  Container *ns = rtems_unified_container_get_namespaces(worker->container);
  Container *root = rtems_container_get_root();
  rtems_task_priority priority;
  rtems_mode modes;
  rtems_status_code sc;

  rtems_test_assert(rtems_scheduler_get_processor() == 0);
  sc = rtems_task_set_priority(RTEMS_SELF, RTEMS_CURRENT_PRIORITY, &priority);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert(priority == C07_WORKER_PRIORITY);
  sc = rtems_task_mode(0, 0, &modes);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert((modes & RTEMS_PREEMPT_MASK) == RTEMS_PREEMPT);
  rtems_test_assert((modes & RTEMS_TIMESLICE_MASK) == RTEMS_NO_TIMESLICE);
  rtems_test_assert(worker->container->attached_thread == self);
  rtems_test_assert(self->cgroup ==
    rtems_unified_container_get_core_cgroup(worker->container));
  rtems_test_assert(self->cgroup->cpu_quota_available != 0);
  rtems_test_assert(self->container->pidContainer == ns->pidContainer);
  rtems_test_assert(self->container->ipcContainer == ns->ipcContainer);
  rtems_test_assert(self->container->mntContainer == ns->mntContainer);
  rtems_test_assert(self->container->netContainer == ns->netContainer);
  rtems_test_assert(self->container->utsContainer == ns->utsContainer);
  rtems_test_assert(ns->pidContainer != root->pidContainer);
  rtems_test_assert(ns->ipcContainer != root->ipcContainer);
  rtems_test_assert(ns->mntContainer != root->mntContainer);
  rtems_test_assert(ns->netContainer != root->netContainer);
  rtems_test_assert(ns->utsContainer != root->utsContainer);
}

static void finish_worker(Worker *worker, rtems_event_set done)
{
  rtems_status_code sc;

  verify_worker(worker);
  sc = rtems_unified_container_leave(
    worker->container, _Thread_Get_executing()
  );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  /* Init has higher priority and may delete this task during event_send. */
  send_events(init_task, done);
  sc = rtems_task_suspend(RTEMS_SELF);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert(false);
}

static void delete_worker(Worker *worker)
{
  rtems_status_code sc;

  rtems_test_assert(!worker->container->attached);
  sc = rtems_task_delete(worker->task);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  sc = rtems_unified_container_delete(worker->container);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  worker->container = NULL;
  worker->task = 0;
}

static rtems_task startup_worker(rtems_task_argument arg)
{
  rtems_status_code sc;

  (void) arg;
  sc = attach_self(&worker_a);
  startup_ready = now_ns(); /* Workload entry: isolation is established. */
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  verify_worker(&worker_a);
  send_events(init_task, READY_A);
  receive_events(FINISH, normal_timeout());
  finish_worker(&worker_a, DONE_A);
}

static void measure_startup(uint32_t round, unsigned int mode)
{
  uint32_t i;

  rtems_test_assert(mode <= 1u);
  for (i = 0; i < C07_STARTUP_WARMUP + C07_STARTUP_SAMPLES; ++i) {
    StartupSample sample = {0, 0, 0};
    rtems_status_code sc;

    startup_ready = 0;
    if (mode == 0) {
      sample.begin = now_ns();
    }
    create_container(&worker_a);
    create_task(&worker_a, rtems_build_name('S', 'T', '0', '7'));
    if (mode == 1) {
      sample.begin = now_ns();
    }
    sc = rtems_task_start(worker_a.task, startup_worker, 0);
    sample.api_return = now_ns();
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    receive_events(READY_A, normal_timeout());
    sample.ready = startup_ready;
    rtems_test_assert(sample.ready >= sample.begin);
    rtems_test_assert(sample.api_return >= sample.begin);
    if (i >= C07_STARTUP_WARMUP) {
      startup_samples[round][mode][i - C07_STARTUP_WARMUP] = sample;
    }
    send_events(worker_a.task, FINISH);
    receive_events(DONE_A, normal_timeout());
    delete_worker(&worker_a);
  }
}

static rtems_task timer_worker(rtems_task_argument arg)
{
  rtems_status_code sc;
  uint64_t deadline;
  uint32_t i;

  (void) arg;
  sc = attach_self(&worker_a);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  verify_worker(&worker_a);
  send_events(init_task, READY_A);
  receive_events(GO, normal_timeout());
  deadline = now_ns() + UINT64_C(100000000);

  for (i = 0; i < C07_TIMER_WARMUP + C07_TIMER_SAMPLES; ++i) {
    struct timespec absolute;
    uint64_t actual;
    int eno;

    deadline += C07_PERIOD_NS;
    absolute.tv_sec = (time_t) (deadline / C07_NS_PER_SECOND);
    absolute.tv_nsec = (long) (deadline % C07_NS_PER_SECOND);
    do {
      eno = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &absolute, NULL);
    } while (eno == EINTR);
    actual = now_ns();
    rtems_test_assert(eno == 0);
    rtems_test_assert(actual >= deadline);
    if (i >= C07_TIMER_WARMUP) {
      uint32_t index = i - C07_TIMER_WARMUP;
      uint64_t missed = (actual - deadline) / C07_PERIOD_NS;

      timer_samples[worker_a.round][index] = (TimerSample) {deadline, actual};
      if (missed != 0) {
        ++timer_overruns[worker_a.round];
      }
      if (missed > timer_max_missed_periods[worker_a.round]) {
        timer_max_missed_periods[worker_a.round] = missed;
      }
    }
  }
  finish_worker(&worker_a, DONE_A);
}

static void measure_timer(uint32_t round)
{
  rtems_status_code sc;
  uint64_t allowed_ns =
    ((uint64_t) C07_TIMER_WARMUP + C07_TIMER_SAMPLES) * C07_PERIOD_NS +
    (uint64_t) C07_WAIT_SECONDS * C07_NS_PER_SECOND;

  rtems_test_assert(allowed_ns <
    (uint64_t) C07_CPU_BUDGET_TICKS * C07_TICK_US * 1000u);
  worker_a.round = round;
  create_container(&worker_a);
  create_task(&worker_a, rtems_build_name('T', 'M', '0', '7'));
  sc = rtems_task_start(worker_a.task, timer_worker, 0);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  receive_events(READY_A, normal_timeout());
  send_events(worker_a.task, GO);
  receive_events(DONE_A, timeout_ticks(allowed_ns));
  delete_worker(&worker_a);
}

static rtems_task switch_receiver(rtems_task_argument arg)
{
  rtems_status_code sc;
  rtems_interval timeout = normal_timeout();
  uint32_t i;

  (void) arg;
  sc = attach_self(&worker_b);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  verify_worker(&worker_b);
  send_events(init_task, READY_B);
  for (i = 0; i < C07_SWITCH_WARMUP + C07_SWITCH_SAMPLES; ++i) {
    rtems_event_set received;
    uint64_t received_ns;

    sc = rtems_event_receive(PING, RTEMS_EVENT_ALL | RTEMS_WAIT,
      timeout, &received);
    received_ns = now_ns();
    rtems_test_assert(sc == RTEMS_SUCCESSFUL && received == PING);
    rtems_test_assert(request_sequence == i + 1u);
    peer_received = received_ns;
    reply_sequence = i + 1u;
    peer_replied = now_ns();
    send_events(worker_a.task, PONG);
  }
  /* Stay in the container until A has timestamped the final response. */
  receive_events(FINISH, timeout);
  finish_worker(&worker_b, DONE_B);
}

static rtems_task switch_sender(rtems_task_argument arg)
{
  rtems_status_code sc;
  rtems_interval timeout = normal_timeout();
  uint32_t i;

  (void) arg;
  sc = attach_self(&worker_a);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  verify_worker(&worker_a);
  send_events(init_task, READY_A);
  receive_events(GO, timeout);

  for (i = 0; i < C07_SWITCH_WARMUP + C07_SWITCH_SAMPLES; ++i) {
    SwitchSample sample;
    rtems_event_set received;
    rtems_status_code sent_sc;

    request_sequence = i + 1u;
    sample.sent = now_ns();
    sent_sc = rtems_event_send(worker_b.task, PING);
    sc = rtems_event_receive(PONG, RTEMS_EVENT_ALL | RTEMS_WAIT,
      timeout, &received);
    sample.returned = now_ns();
    sample.peer_received = peer_received;
    sample.peer_replied = peer_replied;
    rtems_test_assert(sent_sc == RTEMS_SUCCESSFUL);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL && received == PONG);
    rtems_test_assert(reply_sequence == i + 1u);
    rtems_test_assert(sample.sent <= sample.peer_received);
    rtems_test_assert(sample.peer_received <= sample.peer_replied);
    rtems_test_assert(sample.peer_replied <= sample.returned);
    if (i >= C07_SWITCH_WARMUP) {
      switch_samples[worker_a.round][i - C07_SWITCH_WARMUP] = sample;
    }
  }
  send_events(worker_b.task, FINISH);
  finish_worker(&worker_a, DONE_A);
}

static void measure_switch(uint32_t round)
{
  rtems_status_code sc;

  worker_a.round = round;
  worker_b.round = round;
  request_sequence = 0;
  reply_sequence = 0;
  create_container(&worker_a);
  create_container(&worker_b);
  rtems_test_assert(worker_a.container->core_cgroup !=
    worker_b.container->core_cgroup);
  rtems_test_assert(worker_a.container->namespaces.pidContainer !=
    worker_b.container->namespaces.pidContainer);
  create_task(&worker_a, rtems_build_name('T', 'X', '0', '7'));
  create_task(&worker_b, rtems_build_name('R', 'X', '0', '7'));
  sc = rtems_task_start(worker_b.task, switch_receiver, 0);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  sc = rtems_task_start(worker_a.task, switch_sender, 0);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  receive_events(READY_A | READY_B, normal_timeout());
  send_events(worker_a.task, GO);
  receive_events(DONE_A | DONE_B, timeout_ticks(
    2u * (uint64_t) C07_WAIT_SECONDS * C07_NS_PER_SECOND));
  delete_worker(&worker_a);
  delete_worker(&worker_b);
}

static uint32_t samples_per_round(Metric metric)
{
  if (metric <= START_API) {
    return C07_STARTUP_SAMPLES;
  }
  return metric == TIMER_WAKEUP ? C07_TIMER_SAMPLES : C07_SWITCH_SAMPLES;
}

static uint64_t sample_value(Metric metric, uint32_t round, uint32_t index)
{
  if (metric <= START_API) {
    unsigned int mode = (metric == START_READY || metric == START_API) ? 1 : 0;
    const StartupSample *s = &startup_samples[round][mode][index];
    return (metric <= START_READY ? s->ready : s->api_return) - s->begin;
  }
  if (metric == TIMER_WAKEUP) {
    const TimerSample *s = &timer_samples[round][index];
    return s->actual - s->deadline;
  }
  {
    const SwitchSample *s = &switch_samples[round][index];
    if (metric == SWITCH_A_TO_B) {
      return s->peer_received - s->sent;
    }
    if (metric == SWITCH_B_TO_A) {
      return s->returned - s->peer_replied;
    }
    /* Keep full RTT integers.  Divide by 2.0 at reporting, not before sorting. */
    return s->returned - s->sent;
  }
}

static int compare_u64(const void *left, const void *right)
{
  uint64_t a = *(const uint64_t *) left;
  uint64_t b = *(const uint64_t *) right;
  return (a > b) - (a < b);
}

static double percentile(size_t count, unsigned int percent, double scale)
{
  size_t rank = (count * percent + 99u) / 100u;
  return (double) sort_buffer[rank - 1u] * scale;
}

static void print_summary(Metric metric, uint32_t first, uint32_t end,
  uint32_t report_round)
{
  size_t count = 0;
  size_t j;
  uint32_t round;
  uint32_t i;
  double mean = 0.0;
  double m2 = 0.0;
  double scale = metric == SWITCH_HALF_RTT ? 0.5 : 1.0;

  for (round = first; round < end; ++round) {
    for (i = 0; i < samples_per_round(metric); ++i) {
      rtems_test_assert(count < C07_SORT_CAPACITY);
      sort_buffer[count++] = sample_value(metric, round, i);
    }
  }
  for (j = 0; j < count; ++j) {
    double value = (double) sort_buffer[j] * scale;
    double delta = value - mean;
    mean += delta / (double) (j + 1u);
    m2 += delta * (value - mean);
  }
  qsort(sort_buffer, count, sizeof(sort_buffer[0]), compare_u64);
  printf("C07SUMMARY,%s,%" PRIu32 ",%zu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
    metric_names[metric], report_round, count, (double) sort_buffer[0] * scale,
    mean, percentile(count, 50, scale), percentile(count, 95, scale),
    percentile(count, 99, scale), (double) sort_buffer[count - 1u] * scale,
    count > 1u ? sqrt(m2 / (double) (count - 1u)) : 0.0);
}

static void print_raw(void)
{
#if C07_PRINT_RAW
  uint32_t round;
  uint32_t i;
  unsigned int mode;

  puts("C07RAW_HEADER,kind,round,sample,t0_ns,t1_ns,t2_ns,t3_ns");
  for (round = 0; round < C07_ROUNDS; ++round) {
    for (mode = 0; mode < 2; ++mode) {
      for (i = 0; i < C07_STARTUP_SAMPLES; ++i) {
        const StartupSample *s = &startup_samples[round][mode][i];
        printf("C07RAW,%s,%" PRIu32 ",%" PRIu32 ",%" PRIu64
          ",%" PRIu64 ",%" PRIu64 ",0\n",
          mode == 0 ? "run" : "start", round + 1u, i,
          s->begin, s->ready, s->api_return);
      }
    }
    for (i = 0; i < C07_TIMER_SAMPLES; ++i) {
      const TimerSample *s = &timer_samples[round][i];
      printf("C07RAW,timer,%" PRIu32 ",%" PRIu32 ",%" PRIu64
        ",%" PRIu64 ",0,0\n", round + 1u, i, s->deadline, s->actual);
    }
    for (i = 0; i < C07_SWITCH_SAMPLES; ++i) {
      const SwitchSample *s = &switch_samples[round][i];
      printf("C07RAW,switch,%" PRIu32 ",%" PRIu32 ",%" PRIu64
        ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 "\n",
        round + 1u, i, s->sent, s->peer_received, s->peer_replied, s->returned);
    }
  }
#endif
}

static void print_metadata(void)
{
  struct timespec resolution;
  int resolution_status = clock_getres(CLOCK_MONOTONIC, &resolution);

  puts("C07META,clock,CLOCK_MONOTONIC");
  puts("C07META,scheduler,RTEMS_PRIORITY_PREEMPT_NO_TIMESLICE");
  puts("C07META,cpu,0");
  printf("C07META,priority,%u\n", C07_WORKER_PRIORITY);
  printf("C07META,container_flags,%u\n", (unsigned int) RTEMS_UNIFIED_CONTAINER_ALL);
  printf("C07META,rounds,%u\n", C07_ROUNDS);
  printf("C07META,startup_samples,%u\n", C07_STARTUP_SAMPLES);
  printf("C07META,startup_warmup,%u\n", C07_STARTUP_WARMUP);
  printf("C07META,timer_samples,%u\n", C07_TIMER_SAMPLES);
  printf("C07META,timer_warmup,%u\n", C07_TIMER_WARMUP);
  printf("C07META,switch_samples,%u\n", C07_SWITCH_SAMPLES);
  printf("C07META,switch_warmup,%u\n", C07_SWITCH_WARMUP);
  printf("C07META,period_us,%u\n", C07_PERIOD_US);
  printf("C07META,tick_us,%u\n", C07_TICK_US);
  if (resolution_status == 0) {
    printf("C07META,clock_getres_monotonic_ns,%" PRIu64 "\n",
      (uint64_t) resolution.tv_sec * C07_NS_PER_SECOND +
      (uint64_t) resolution.tv_nsec);
  } else {
    /* This RTEMS branch supports MONOTONIC reads/sleeps, but not getres. */
    puts("C07META,clock_getres_monotonic_ns,unsupported");
  }
  printf("C07META,counter_frequency_hz,%" PRIu32 "\n",
    rtems_counter_frequency());
  printf("C07META,raw,%d\n", C07_PRINT_RAW);
  printf("C07META,cpu_budget_ticks,%u\n", C07_CPU_BUDGET_TICKS);
#ifdef RTEMSCFG_CONTAINER_LOG
  puts("C07META,container_logging,1");
#else
  puts("C07META,container_logging,0");
#endif
#ifdef RTEMSCFG_MONITOR_CPU
  puts("C07META,cpu_monitor,1");
#else
  puts("C07META,cpu_monitor,0");
#endif
#ifdef RTEMSCFG_MONITOR_MEM
  puts("C07META,memory_monitor,1");
#else
  puts("C07META,memory_monitor,0");
#endif
#ifdef RTEMSCFG_MONITOR_NET
  puts("C07META,network_monitor,1");
#else
  puts("C07META,network_monitor,0");
#endif
}

static rtems_task Init(rtems_task_argument arg)
{
  uint32_t round;
  Metric metric;

  (void) arg;
  TEST_BEGIN();
  init_task = rtems_task_self();
  pin_to_cpu_zero(init_task);
  rtems_test_assert(rtems_scheduler_get_processor() == 0);
  rtems_test_assert(rtems_clock_get_ticks_per_second() ==
    1000000u / C07_TICK_US);
  /* Global network initialization is not charged to an individual container. */
  rtems_test_assert(rtems_bsdnet_initialize_network() == 0);
  print_metadata();
  memset(startup_samples, 0, sizeof(startup_samples));
  memset(timer_samples, 0, sizeof(timer_samples));
  memset(switch_samples, 0, sizeof(switch_samples));
  memset(sort_buffer, 0, sizeof(sort_buffer));

  for (round = 0; round < C07_ROUNDS; ++round) {
    printf("[container07] round %" PRIu32 "/%u: startup\n", round + 1u, C07_ROUNDS);
    measure_startup(round, 0);
    measure_startup(round, 1);
    printf("[container07] round %" PRIu32 "/%u: timer\n", round + 1u, C07_ROUNDS);
    measure_timer(round);
    printf("[container07] round %" PRIu32 "/%u: cross-container handoff\n",
      round + 1u, C07_ROUNDS);
    measure_switch(round);
  }

  puts("C07SUMMARY_HEADER,metric,round,n,min_ns,mean_ns,p50_ns,p95_ns,p99_ns,max_ns,stddev_ns");
  for (metric = RUN_READY; metric < METRIC_COUNT; ++metric) {
    for (round = 0; round < C07_ROUNDS; ++round) {
      print_summary(metric, round, round + 1u, round + 1u);
    }
    print_summary(metric, 0, C07_ROUNDS, 0); /* round 0 = pooled */
  }
  for (round = 0; round < C07_ROUNDS; ++round) {
    printf("C07TIMER,%" PRIu32 ",%" PRIu64 ",%" PRIu64 "\n",
      round + 1u, timer_overruns[round], timer_max_missed_periods[round]);
  }
  print_raw();
  puts("C07DONE");
  TEST_END();
  rtems_test_exit(0);
}

struct rtems_bsdnet_config rtems_bsdnet_config = {
  NULL, NULL, 0, 0, 0, 0, 0, 0, 0,
  {"0.0.0.0"}, {"0.0.0.0"}, 0, 0, 0, 0, 0
};

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_APPLICATION_NEEDS_LIBNETWORKING
#define CONFIGURE_MICROSECONDS_PER_TICK C07_TICK_US
#define CONFIGURE_MAXIMUM_PROCESSORS 1
#define CONFIGURE_SCHEDULER_PRIORITY
#define CONFIGURE_MAXIMUM_TASKS 12
#define CONFIGURE_MAXIMUM_CGROUPS 2
#define CONFIGURE_MAXIMUM_SEMAPHORES 16
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 4
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 20
#define CONFIGURE_MESSAGE_BUFFER_MEMORY \
  CONFIGURE_MESSAGE_BUFFERS_FOR_QUEUE(4, sizeof(uint32_t))
#define CONFIGURE_EXECUTIVE_RAM_SIZE (16u * 1024u * 1024u)
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_PRIORITY 1
#define CONFIGURE_INIT_TASK_INITIAL_MODES (RTEMS_PREEMPT | RTEMS_NO_TIMESLICE)
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_INIT_TASK_STACK_SIZE (8u * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
