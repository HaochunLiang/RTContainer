#ifndef _RTEMS_SCORE_NETCONTAINER_H
#define _RTEMS_SCORE_NETCONTAINER_H
#include <rtems/score/object.h>
#include <stdint.h>

struct ifnet; 
struct ifaddr; 
struct rtems_bsdnet_ifconfig;

#ifdef RTEMSCFG_Carousel_CONTAINER
struct mbuf;

typedef struct carousel carousel_t;

/* Carousel调度器核心API */
carousel_t *carousel_init(uint64_t default_rate_bps);
void carousel_destroy(carousel_t *carousel);
int carousel_enqueue_packet(
  carousel_t *carousel,
  struct mbuf *m,
  uint32_t src_ip,
  uint32_t dst_ip,
  uint16_t src_port,
  uint16_t dst_port,
  uint8_t protocol
);
int carousel_set_flow_rate(
  carousel_t *carousel,
  uint32_t src_ip,
  uint32_t dst_ip,
  uint16_t src_port,
  uint16_t dst_port,
  uint8_t protocol,
  uint64_t rate_bps
);
void carousel_process(carousel_t *carousel, struct ifnet *ifp);

/* Carousel统计信息API */
void carousel_get_stats(
  carousel_t *carousel,
  uint32_t *total_flows,
  uint64_t *total_packets_sent,
  uint64_t *total_packets_dropped
);
#endif

/* 前向声明 PCB 相关结构 */
struct  inpcbhead;
struct inpcbinfo;
struct veth_softc;
struct in_ifaddr;  /* IPv4 接口地址结构 */

typedef struct net_group
{
    // 接口
    // struct ifnet *ifnet_list;
    struct ifnet *ifnet_p;
    struct ifaddr **ifnet_addrs;
    int if_index_counter;
    int if_indexlim;
    struct rtems_bsdnet_ifconfig *ifnet_config;
    
    // IPv4 地址管理
    struct in_ifaddr *in_ifaddr;       /* IPv4 接口地址链表头 */
    
    // 路由表
    struct radix_node_head *rt_tables[32];  /* AF_MAX+1 */
    
    // PCB (端口控制块) - 用于端口隔离
    struct inpcbhead *udp_pcblist;     /* UDP PCB 链表头 */
    struct inpcbinfo *udp_pcbinfo;     /* UDP PCB 信息 */
    struct inpcbhead *tcp_pcblist;     /* TCP PCB 链表头 */
    struct inpcbinfo *tcp_pcbinfo;     /* TCP PCB 信息 */
    
    // veth 虚拟网络接口
    struct veth_softc *veth_sc;        /* veth 接口控制结构 */
    
#ifdef RTEMSCFG_Carousel_CONTAINER
    carousel_t *carousel;          /* Carousel流调度器实例 */
    uint64_t carousel_default_rate; /* 默认速率限制(bps) */
    bool carousel_enabled;         /* Carousel是否启用 */
#endif
    
    // 路由
    //...
} net_group;
typedef struct NetContainer
{
    int rc;
    struct net_group *group; // 之后再确定有什么量在里面
    int containerID;
} NetContainer;

typedef struct _Thread_Control Thread_Control;

// 初始化根net容器
int rtems_net_container_initialize_root(NetContainer **netContainer);

// 创建子net容器
NetContainer *rtems_net_container_create(void);

// 删除子net容器
void rtems_net_container_delete(NetContainer *netContainer);

// 将net容器添加到链表中
void rtems_net_container_add_to_list(NetContainer *netContainer);

// 从链表中移除net容器
void rtems_net_container_remove_from_list(NetContainer *netContainer);

// 移动任务到指定的net容器
void rtems_net_container_move_task(NetContainer *srcContainer, NetContainer *destContainer, Thread_Control *thread);

// 获取容器ID
int rtems_net_container_get_id(NetContainer *netContainer);

// 获取容器引用计数
int rtems_net_container_get_rc(NetContainer *netContainer);

int rtems_net_container_get_NetContainerNum(void);
struct ifnet *rtems_net_container_get_ifnet(void);
struct ifaddr **rtems_net_container_get_ifnet_addrs(void);

/* 获取当前容器的 IPv4 地址链表 */
struct in_ifaddr **rtems_net_container_get_in_ifaddr(void);

/* 获取当前容器的路由表 */
struct radix_node_head **rtems_net_container_get_rt_tables(void);

/* 获取当前容器的 UDP PCB 链表和信息 */
struct inpcbhead *rtems_net_container_get_udp_pcblist(void);
struct inpcbinfo *rtems_net_container_get_udp_pcbinfo(void);

#ifdef RTEMSCFG_Carousel_CONTAINER
/* Carousel集成API - 容器级别流量控制 */

/* 为网络容器启用Carousel流调度器 */
int rtems_net_container_enable_carousel(
  NetContainer *netContainer,
  uint64_t default_rate_bps  /* 默认速率限制，0表示使用10Mbps */
);

/* 禁用容器的Carousel调度器 */
void rtems_net_container_disable_carousel(NetContainer *netContainer);

/* 检查容器是否启用了Carousel */
bool rtems_net_container_is_carousel_enabled(NetContainer *netContainer);

/* 为容器中的特定流设置速率 */
int rtems_net_container_set_flow_rate(
  NetContainer *netContainer,
  uint32_t src_ip,
  uint32_t dst_ip,
  uint16_t src_port,
  uint16_t dst_port,
  uint8_t protocol,
  uint64_t rate_bps
);

/* 获取容器的Carousel统计信息 */
int rtems_net_container_get_carousel_stats(
  NetContainer *netContainer,
  uint32_t *total_flows,
  uint64_t *total_packets_sent,
  uint64_t *total_packets_dropped
);
#endif

#endif