#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/hpet.h>
#include <linux/io.h>
#include <linux/skbuff.h>

#include <asm/hpet.h>

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



/* hpet counter */
extern void __iomem *hpet_virt_address;
static inline unsigned long long int get_hpet_counter(void)
{
	return readq(hpet_virt_address + HPET_COUNTER);
}

void logging_net_stack(unsigned int func, int cpu, struct sk_buff *skb);

void debug_print_nsl_table(void);

