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
#define NSL_NET_RX_ACTION     1
#define NSL_NETIF_RECEIVE_SKB 2
#define NSL_IP_RCV            3
#define NSL_UDP_RCV           4
#define NSL_UDP_RECVMSG       5

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
extern struct net_stack_log **nsl_table;
extern atomic_t nsl_index[];
extern void __iomem *hpet_virt_address;

static inline void logging_net_stack(unsigned int func, struct sk_buff *skb)
{
	int cpu = smp_processor_id();
	int index;

	if (nsl_enable &&
	    (index = atomic_inc_return(&nsl_index[cpu])) < NSL_LOG_SIZE) {
		struct iphdr *ip;
		struct transport_port *tp_port;
		unsigned int mhdr;
	
		mhdr = skb->mac_header + MAC_HEADER_LEN;
		ip = (struct iphdr *)((char *)skb->head + mhdr);
		tp_port = (struct transport_port *)
			((char *)skb->head + mhdr + (ip->ihl * 4));

		nsl_table[cpu][index].func         = func;
		nsl_table[cpu][index].eth_protocol = skb->protocol;
		nsl_table[cpu][index].ip_protocol  = ip->protocol;
		nsl_table[cpu][index].ip_saddr     = ip->saddr;
		nsl_table[cpu][index].ip_daddr     = ip->daddr;
		nsl_table[cpu][index].tp_sport     = tp_port->sport;
		nsl_table[cpu][index].tp_dport     = tp_port->dport;
		nsl_table[cpu][index].time         = 
			readq(hpet_virt_address + HPET_COUNTER);
		nsl_table[cpu][index].skb         = (uint64_t)skb;
	}
}
