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

#define NSL_GET_INDEX _IO(NSL_MAJOR, 0)
#define NSL_GET_TABLE _IOW(NSL_MAJOR, 1, void *)

#define NSL_LOG_SIZE 1048576

// NSL : net stack log 
// func ID
#define NSL_NET_RX_ACTION     0
#define NSL_NETIF_RECEIVE_SKB 1
#define NSL_IP_RCV            2
#define NSL_UDP_RCV           3
#define NSL_UDP_RECVMSG       4

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
	uint32_t cpu;
	uint16_t eth_protocol;
	uint8_t  ip_protocol;
	uint32_t ip_saddr;
	uint32_t ip_daddr;
	uint16_t tp_sport;
	uint16_t tp_dport;
	uint64_t time;
};

extern struct net_stack_log nsl_table[];
extern atomic_t atomic_index;
extern void __iomem *hpet_virt_address;

static inline void logging_net_stack(unsigned int func, int cpu, struct sk_buff *skb)
{
	int index;

	if ((index = atomic_inc_return(&atomic_index)) < NSL_LOG_SIZE) {
		struct iphdr *ip;
		struct transport_port *tp_port;
		unsigned int mhdr;
	
		mhdr = skb->mac_header + MAC_HEADER_LEN;
		ip = (struct iphdr *)((char *)skb->head + mhdr);
		tp_port = (struct transport_port *)
			((char *)skb->head + mhdr + (ip->ihl * 4));

		nsl_table[index].func         = func;
		nsl_table[index].cpu          = cpu;
		nsl_table[index].eth_protocol = skb->protocol;
		nsl_table[index].ip_protocol  = ip->protocol;
		nsl_table[index].ip_saddr     = ip->saddr;
		nsl_table[index].ip_daddr     = ip->daddr;
		nsl_table[index].tp_sport     = tp_port->sport;
		nsl_table[index].tp_dport     = tp_port->dport;
		nsl_table[index].time         = 
			readq(hpet_virt_address + HPET_COUNTER);
	}
}
