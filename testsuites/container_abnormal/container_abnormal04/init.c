/* SPDX-License-Identifier: BSD-2-Clause */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../support.h"
#include <rtems/bdbuf.h>
#include <rtems/blkdev.h>
#include <rtems/io_cgroup.h>
#include <rtems/ramdisk.h>
#include <rtems/score/container.h>
#include <rtems/score/threadimpl.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <string.h>
#include <unistd.h>

const char rtems_test_name[] = "CONTAINER ABNORMAL 04";

#define BLOCK_SIZE 512
#define BLOCK_COUNT 32
#define WRITE_BLOCK RTEMS_EVENT_0
#define READ_BLOCK RTEMS_EVENT_1
#define STOP RTEMS_EVENT_2
#define ACK(i) (RTEMS_EVENT_0 << (i))

typedef struct {
  const char *path;
  ramdisk *ram;
  rtems_disk_device *disk;
  int fd;
  RtemsContainer *container;
  rtems_id worker_id;
  rtems_status_code result;
  unsigned char pattern;
} DiskContext;

static DiskContext disks[2] = {
  {.path = "/dev/abnormal-a"}, {.path = "/dev/abnormal-b"}
};
static rtems_id manager_id;
static atomic_bool device_unavailable;
static atomic_uint failed_reads;

static int fault_disk_ioctl(rtems_disk_device *dd, uint32_t request, void *arg)
{
  ramdisk *rd = rtems_disk_get_driver_data(dd);
  if (request == RTEMS_BLKIO_REQUEST) {
    rtems_blkdev_request *io = arg;
    if (rd == disks[0].ram && atomic_load(&device_unavailable) &&
        io->req == RTEMS_BLKDEV_REQ_READ) {
      atomic_fetch_add(&failed_reads, 1);
      /* Complete the request with an error; omitting completion would hang bdbuf. */
      rtems_blkdev_request_done(io, RTEMS_IO_ERROR);
      return 0;
    }
  } else if (request == RTEMS_BLKIO_DELETED) {
    ramdisk_free(rd);
    return 0;
  }
  return ramdisk_ioctl(dd, request, arg);
}

static void acknowledge(unsigned index)
{
  rtems_test_assert(rtems_event_send(manager_id, ACK(index)) == RTEMS_SUCCESSFUL);
}

static void wait_ack(unsigned index)
{
  rtems_event_set received;
  rtems_test_assert(rtems_event_receive(
    ACK(index), RTEMS_EVENT_ALL | RTEMS_WAIT,
    2 * rtems_clock_get_ticks_per_second(), &received
  ) == RTEMS_SUCCESSFUL);
}

static rtems_task io_worker(rtems_task_argument index)
{
  DiskContext *ctx = &disks[index];
  rtems_event_set received;
  rtems_bdbuf_buffer *buffer;
  IO_Cgroup_Request request;
  size_t i;

  rtems_test_assert(rtems_unified_container_enter(
    ctx->container, _Thread_Get_executing()
  ) == RTEMS_SUCCESSFUL);
  acknowledge(index);
  for (;;) {
    rtems_test_assert(rtems_event_receive(
      WRITE_BLOCK | READ_BLOCK | STOP, RTEMS_EVENT_ANY | RTEMS_WAIT,
      20 * rtems_clock_get_ticks_per_second(), &received
    ) == RTEMS_SUCCESSFUL);
    if ((received & STOP) != 0) {
      break;
    }
    memset(&request, 0, sizeof(request));
    request.device = ctx->disk->dev;
    request.block = 0;
    request.size = BLOCK_SIZE;
    request.type = (received & WRITE_BLOCK) != 0 ? IO_CGROUP_WRITE : IO_CGROUP_READ;
    request.timestamp = rtems_clock_get_ticks_since_boot();
    /* This API admits/accounts requests; bdbuf performs the actual device IO. */
    rtems_test_assert(rtems_io_cgroup_handle_request(
      ctx->container->io_cgroup, &request
    ) == RTEMS_SUCCESSFUL);
    if ((received & WRITE_BLOCK) != 0) {
      rtems_test_assert(rtems_bdbuf_get(ctx->disk, 0, &buffer) == RTEMS_SUCCESSFUL);
      memset(buffer->buffer, ctx->pattern, BLOCK_SIZE);
      ctx->result = rtems_bdbuf_sync(buffer);
    } else {
      /* Force a driver request; cached data must not mask the injected fault. */
      rtems_bdbuf_purge_dev(ctx->disk);
      buffer = NULL;
      ctx->result = rtems_bdbuf_read(ctx->disk, 0, &buffer);
      if (ctx->result == RTEMS_SUCCESSFUL) {
        for (i = 0; i < BLOCK_SIZE; ++i) {
          if (((unsigned char *) buffer->buffer)[i] != ctx->pattern) {
            CHECK(false);
            break;
          }
        }
        rtems_test_assert(rtems_bdbuf_release(buffer) == RTEMS_SUCCESSFUL);
      }
    }
    acknowledge(index);
  }
  rtems_test_assert(rtems_unified_container_leave(
    ctx->container, _Thread_Get_executing()
  ) == RTEMS_SUCCESSFUL);
  acknowledge(index);
  rtems_task_exit();
}

static void request_io(unsigned index, rtems_event_set operation, rtems_status_code expected)
{
  rtems_test_assert(rtems_event_send(disks[index].worker_id, operation) == RTEMS_SUCCESSFUL);
  wait_ack(index);
  printf("[io] %s %s: %s\n", disks[index].path,
    operation == WRITE_BLOCK ? "write+sync" : "uncached read",
    rtems_status_text(disks[index].result));
  CHECK(disks[index].result == expected);
}

static rtems_task Init(rtems_task_argument arg)
{
  RtemsContainerConfig config;
  unsigned i;
  unsigned round;
  uint64_t healthy_read_bytes;

  (void) arg;
  abnormal_begin();
  manager_id = rtems_task_self();
  rtems_unified_container_config_initialize(&config);
  config.flags = RTEMS_UNIFIED_CONTAINER_IO;
  config.io_system_read_bps = 1024 * 1024 * 1024;
  config.io_system_write_bps = 1024 * 1024 * 1024;
  config.io_read_bps_limit = 1024 * 1024 * 1024;
  config.io_write_bps_limit = 1024 * 1024 * 1024;
  for (i = 0; i < 2; ++i) {
    disks[i].ram = ramdisk_allocate(NULL, BLOCK_SIZE, BLOCK_COUNT, false);
    rtems_test_assert(disks[i].ram != NULL);
    rtems_test_assert(rtems_blkdev_create(
      disks[i].path, BLOCK_SIZE, BLOCK_COUNT, fault_disk_ioctl, disks[i].ram
    ) == RTEMS_SUCCESSFUL);
    disks[i].fd = open(disks[i].path, O_RDWR);
    rtems_test_assert(disks[i].fd >= 0);
    rtems_test_assert(rtems_disk_fd_get_disk_device(disks[i].fd, &disks[i].disk) == 0);
    rtems_test_assert(rtems_unified_container_create(
      &config, &disks[i].container
    ) == RTEMS_SUCCESSFUL);
    rtems_test_assert(rtems_task_create(
      rtems_build_name('I', 'O', 'A', '0' + i), 9,
      2 * RTEMS_MINIMUM_STACK_SIZE, RTEMS_DEFAULT_MODES,
      RTEMS_DEFAULT_ATTRIBUTES, &disks[i].worker_id
    ) == RTEMS_SUCCESSFUL);
    rtems_test_assert(rtems_task_start(disks[i].worker_id, io_worker, i) == RTEMS_SUCCESSFUL);
    wait_ack(i);
    disks[i].pattern = 0x30 + i;
    request_io(i, WRITE_BLOCK, RTEMS_SUCCESSFUL);
    request_io(i, READ_BLOCK, RTEMS_SUCCESSFUL);
  }
  rtems_test_assert(disks[0].container->io_cgroup != disks[1].container->io_cgroup);
  for (round = 0; round < 3; ++round) {
    atomic_store(&device_unavailable, true);
    healthy_read_bytes = disks[1].container->io_cgroup->stats.read.bytes;
    request_io(0, READ_BLOCK, RTEMS_IO_ERROR);
    CHECK(atomic_load(&failed_reads) == round + 1);
    CHECK(disks[1].container->io_cgroup->stats.read.bytes == healthy_read_bytes);
    ++disks[1].pattern;
    request_io(1, WRITE_BLOCK, RTEMS_SUCCESSFUL);
    request_io(1, READ_BLOCK, RTEMS_SUCCESSFUL);
    CHECK(disks[1].container->io_cgroup->stats.read.bytes == healthy_read_bytes + BLOCK_SIZE);
    atomic_store(&device_unavailable, false);
    /* The failed read must not corrupt the data already stored on disk. */
    request_io(0, READ_BLOCK, RTEMS_SUCCESSFUL);
    ++disks[0].pattern;
    request_io(0, WRITE_BLOCK, RTEMS_SUCCESSFUL);
    request_io(0, READ_BLOCK, RTEMS_SUCCESSFUL);
  }
  for (i = 0; i < 2; ++i) {
    uint32_t id = disks[i].container->io_cgroup_id;
    rtems_test_assert(rtems_event_send(disks[i].worker_id, STOP) == RTEMS_SUCCESSFUL);
    wait_ack(i);
    rtems_test_assert(rtems_unified_container_delete(disks[i].container) == RTEMS_SUCCESSFUL);
    CHECK(rtems_io_cgroup_get_by_id(id) == NULL);
    rtems_bdbuf_purge_dev(disks[i].disk);
    rtems_test_assert(close(disks[i].fd) == 0);
    rtems_test_assert(unlink(disks[i].path) == 0);
  }
  abnormal_finish();
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_MAXIMUM_TASKS 8
#define CONFIGURE_MAXIMUM_SEMAPHORES 12
#define CONFIGURE_MAXIMUM_DRIVERS 4
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 12
#define CONFIGURE_BDBUF_BUFFER_MIN_SIZE BLOCK_SIZE
#define CONFIGURE_BDBUF_BUFFER_MAX_SIZE BLOCK_SIZE
#define CONFIGURE_BDBUF_CACHE_MEMORY_SIZE (32 * 1024)
#define CONFIGURE_BDBUF_MAX_READ_AHEAD_BLOCKS 0
#define CONFIGURE_SWAPOUT_TASK_PRIORITY 8
#define CONFIGURE_EXECUTIVE_RAM_SIZE (4 * 1024 * 1024)
#define CONFIGURE_INIT_TASK_PRIORITY 10
#define CONFIGURE_INIT_TASK_STACK_SIZE (4 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
