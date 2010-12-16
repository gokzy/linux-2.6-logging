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
#define NSL_GET_TABLE _IOWR(NSL_MAJOR, 1, void *)

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

struct transport_port{
	u16 sport;
	u16 dport;
};

struct net_stack_log{
	unsigned int func;
	int cpu;
	u16 eth_protocol;
	u8  ip_protocol;
	u32 ip_saddr;
	u32 ip_daddr;
	u16 tp_sport;
	u16 tp_dport;
	unsigned long long int time;
};

extern struct net_stack_log nsl_table[1000];
extern atomic_t atomic_index;

/* hpet counter */
extern void __iomem *hpet_virt_address;
static inline unsigned long long int get_hpet_counter(void)
{
	return readq(hpet_virt_address + HPET_COUNTER);
}

static inline void logging_net_stack(unsigned int func, int cpu, struct sk_buff *skb)
{

	struct iphdr *ip;
	u32 ihl;
	struct transport_port *tp_port;
	unsigned int mhdr;
	int index;
	
	mhdr = skb->mac_header + MAC_HEADER_LEN;

	ip = (struct iphdr *)((char *)skb->head + mhdr);

	ihl = ip->ihl;
	tp_port = (struct transport_port *)((char *)skb->head + mhdr +(ihl * 4));

	index = atomic_inc_return(&atomic_index);

/* //	if(index < 1000 &&　(ip->protocol == TCP || ip->protocol == UDP)){ */
	if(index < 1000){
		nsl_table[index].func         = func;
		nsl_table[index].cpu          = cpu;
		nsl_table[index].eth_protocol = skb->protocol;
		nsl_table[index].ip_protocol  = ip->protocol;
		nsl_table[index].ip_saddr     = ip->saddr;
		nsl_table[index].ip_daddr     = ip->daddr;
		nsl_table[index].tp_sport     = tp_port->sport;
		nsl_table[index].tp_dport     = tp_port->dport;
		nsl_table[index].time         = get_hpet_counter();
	}

}

void debug_print_nsl_table(void);

