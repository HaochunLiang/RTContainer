/* SPDX-License-Identifier: BSD-2-Clause */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../support.h"
#include <rtems/rtems_bsdnet.h>
#include <rtems/score/container.h>
#include <rtems/score/threadimpl.h>
#define _KERNEL
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/route.h>
#include <netinet/in.h>
#undef _KERNEL
#include <errno.h>
#include <string.h>
#include <unistd.h>

const char rtems_test_name[] = "CONTAINER ABNORMAL 03";

/* Internal stack lock, declared here to avoid its malloc/free macro overrides. */
extern void rtems_bsdnet_semaphore_obtain(void);
extern void rtems_bsdnet_semaphore_release(void);

typedef int (*OutputFunction)(
  struct ifnet *, struct mbuf *, struct sockaddr *, struct rtentry *
);

typedef struct {
  RtemsContainer *container;
  struct ifnet *interface;
  OutputFunction original_output;
  int socket;
} Endpoint;

static Endpoint endpoints[2];
static unsigned failed_packets;

/* A real driver output failure: consume the mbuf and return ENETDOWN. */
static int unavailable_output(
  struct ifnet *ifp, struct mbuf *m, struct sockaddr *dst, struct rtentry *rt
)
{
  (void) dst;
  (void) rt;
  rtems_test_assert(ifp == endpoints[0].interface);
  ++failed_packets;
  ++ifp->if_oerrors;
  m_freem(m);
  return ENETDOWN;
}

static void enter(Endpoint *endpoint)
{
  rtems_test_assert(rtems_unified_container_enter(
    endpoint->container, _Thread_Get_executing()
  ) == RTEMS_SUCCESSFUL);
}

static void leave(Endpoint *endpoint)
{
  rtems_test_assert(rtems_unified_container_leave(
    endpoint->container, _Thread_Get_executing()
  ) == RTEMS_SUCCESSFUL);
}

static struct sockaddr_in address(void)
{
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(18003);
  return addr;
}

static void create_endpoint(Endpoint *endpoint)
{
  RtemsContainerConfig config;
  struct sockaddr_in addr = address();
  struct timeval timeout = {1, 0};
  rtems_unified_container_config_initialize(&config);
  config.flags = RTEMS_UNIFIED_CONTAINER_NET;
  rtems_test_assert(rtems_unified_container_create(
    &config, &endpoint->container
  ) == RTEMS_SUCCESSFUL);
  enter(endpoint);
  endpoint->interface = rtems_net_container_get_ifnet();
  rtems_test_assert(endpoint->interface != NULL);
  rtems_test_assert((endpoint->interface->if_flags & IFF_LOOPBACK) != 0);
  endpoint->original_output = endpoint->interface->if_output;
  rtems_test_assert(endpoint->original_output != NULL);
  endpoint->socket = socket(AF_INET, SOCK_DGRAM, 0);
  rtems_test_assert(endpoint->socket >= 0);
  rtems_test_assert(setsockopt(
    endpoint->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)
  ) == 0);
  rtems_test_assert(setsockopt(
    endpoint->socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)
  ) == 0);
  /* Both live sockets bind the same address/port in separate NET containers. */
  rtems_test_assert(bind(endpoint->socket, (struct sockaddr *) &addr, sizeof(addr)) == 0);
  leave(endpoint);
}

static void exchange(Endpoint *endpoint, uint32_t sequence, bool failing)
{
  struct sockaddr_in addr = address();
  uint32_t reply = 0;
  ssize_t n;
  int error;
  enter(endpoint);
  errno = 0;
  n = sendto(endpoint->socket, &sequence, sizeof(sequence), 0,
    (struct sockaddr *) &addr, sizeof(addr));
  error = errno;
  if (failing) {
    printf("[fault] sendto=%zd errno=%d (expected ENETDOWN=%d)\n", n, error, ENETDOWN);
    CHECK(n == -1 && error == ENETDOWN);
    errno = 0;
    n = recvfrom(endpoint->socket, &reply, sizeof(reply), 0, NULL, NULL);
    error = errno;
    CHECK(n == -1 && (error == EAGAIN || error == EWOULDBLOCK));
  } else {
    rtems_test_assert(n == (ssize_t) sizeof(sequence));
    n = recvfrom(endpoint->socket, &reply, sizeof(reply), 0, NULL, NULL);
    rtems_test_assert(n == (ssize_t) sizeof(reply));
    CHECK(reply == sequence);
    printf("[network] container=%d sequence=%" PRIu32 " received\n",
      endpoint->container->namespaces.netContainer->containerID, sequence);
  }
  leave(endpoint);
}

static void set_fault(bool enabled)
{
  /* Use the same lock as the legacy network stack while changing the driver. */
  rtems_bsdnet_semaphore_obtain();
  endpoints[0].interface->if_output = enabled ?
    unavailable_output : endpoints[0].original_output;
  rtems_bsdnet_semaphore_release();
}

static rtems_task Init(rtems_task_argument arg)
{
  unsigned i;
  struct ifnet *root_interface;
  OutputFunction root_output;
  (void) arg;
  abnormal_begin();
  rtems_test_assert(rtems_bsdnet_initialize_network() == 0);
  root_interface = rtems_net_container_get_ifnet();
  rtems_test_assert(root_interface != NULL);
  root_output = root_interface->if_output;
  for (i = 0; i < 2; ++i) {
    create_endpoint(&endpoints[i]);
    exchange(&endpoints[i], 100 + i, false);
  }
  rtems_test_assert(endpoints[0].interface != endpoints[1].interface);
  rtems_test_assert(endpoints[0].interface != root_interface);
  set_fault(true);
  exchange(&endpoints[0], 200, true);
  CHECK(failed_packets == 1);
  CHECK(endpoints[1].interface->if_output == endpoints[1].original_output);
  exchange(&endpoints[1], 201, false);
  CHECK(root_interface->if_output == root_output);
  set_fault(false);
  /* Reuse the original sockets after recovery. */
  exchange(&endpoints[0], 300, false);
  exchange(&endpoints[1], 301, false);
  CHECK(failed_packets == 1);
  for (i = 0; i < 2; ++i) {
    enter(&endpoints[i]);
    rtems_test_assert(close(endpoints[i].socket) == 0);
    leave(&endpoints[i]);
    rtems_test_assert(rtems_unified_container_delete(
      endpoints[i].container
    ) == RTEMS_SUCCESSFUL);
  }
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
#define CONFIGURE_MAXIMUM_SEMAPHORES 12
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 20
#define CONFIGURE_EXECUTIVE_RAM_SIZE (8 * 1024 * 1024)
#define CONFIGURE_INIT_TASK_PRIORITY 10
#define CONFIGURE_INIT_TASK_STACK_SIZE (4 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION
#define CONFIGURE_INIT
#include <rtems/confdefs.h>
