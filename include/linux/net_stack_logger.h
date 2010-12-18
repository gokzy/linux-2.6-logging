#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/hpet.h>
#include <linux/io.h>
#include <linux/skbuff.h>
#include <linux/ioctl.h>
#include <asm/hpet.h>
#include <linux/ip.h>
#include <asm/atomic.h>
#include <net/ip.h>

#define NSL_DEV_NAME "nsl"
#define NSL_MAJOR 261

#define NSL_GET_TABLE _IOW(NSL_MAJOR, 1, void *)
#define NSL_ENABLE _IO(NSL_MAJOR, 2)
#define NSL_DISABLE _IO(NSL_MAJOR, 3)

#define NSL_LOG_SIZE 1048576
#define NSL_MAX_CPU 8

// NSL : net stack log 
// func ID
#define NSL_NETIF_RECEIVE_SKB_IN  1
#define NSL_NETIF_RECEIVE_SKB_OUT 2

#define NSL___NETIF_RECEIVE_SKB   3
#define NSL_IP_RCV                4
#define NSL_SK_DATA_READY         5

#define NSL_WAIT_FOR_PACKET       6
#define NSL_UDP_RECVMSG           7

#define NSL_TCP_RCV_ESTABLISHED   8
#define NSL_TCP_PREQUEUE_PROCESS  9
#define NSL_TCP_RECVMSG_COPIED   10
#define NSL_TCP_RECVMSG_OUT      11

// protocol 
#define IPv4 0x8
#define TCP  0x6
#define UDP  0x11

#define MAC_HEADER_LEN 14

struct transport_port {
	uint16_t sport;
	uint16_t dport;
};

struct net_stack_log {
	uint32_t func;
	uint16_t eth_protocol;
	uint8_t  ip_protocol;
	uint32_t ip_saddr;
	uint32_t ip_daddr;
	uint16_t tp_sport;
	uint16_t tp_dport;
	uint64_t time;
	uint64_t skb;
};

extern int nsl_enable;
extern struct net_stack_log nsl_table[NSL_MAX_CPU][NSL_LOG_SIZE];
extern atomic_t nsl_index[];
extern void __iomem *hpet_virt_address;

static inline void logging_net_stack(unsigned int func, struct sk_buff *skb)
{
	int cpu = smp_processor_id();
	int index;

	if (nsl_enable &&
	    (index = atomic_inc_return(&nsl_index[cpu])) < NSL_LOG_SIZE) {
		struct iphdr *ip = NULL;
		struct transport_port *tp_port = NULL;
		unsigned int mhdr;

		if (skb && skb->head) {
			mhdr = skb->mac_header + MAC_HEADER_LEN;
			ip = (struct iphdr *)((char *)skb->head + mhdr);
			tp_port = (struct transport_port *)
				((char *)skb->head + mhdr + (ip->ihl * 4));
		}

		nsl_table[cpu][index].func         = func;
		nsl_table[cpu][index].eth_protocol = skb ? skb->protocol : 0;
		nsl_table[cpu][index].ip_protocol  = ip ? ip->protocol : 0;
		nsl_table[cpu][index].ip_saddr     = ip ? ip->saddr : 0;
		nsl_table[cpu][index].ip_daddr     = ip ? ip->daddr : 0;
		nsl_table[cpu][index].tp_sport     = tp_port ? tp_port->sport : 0;
		nsl_table[cpu][index].tp_dport     = tp_port ? tp_port->dport : 0;
		nsl_table[cpu][index].time         = 
			readq(hpet_virt_address + HPET_COUNTER);
		nsl_table[cpu][index].skb         = (uint64_t)skb;
	}
}
