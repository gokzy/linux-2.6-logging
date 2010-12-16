#include <linux/types.h>
#include <linux/skbuff.h>

// NSL : net stack log
#define NSL_NET_RX_ACTION     0
#define NSL_NETIF_RECEIVE_SKB 1
#define NSL_IP_RCV            2
#define NSL_UDP_RCV           3

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
unsigned long long int get_hpet_counter(void);

void logging_net_stack(unsigned int func, int cpu, struct sk_buff *skb);
void debug_print_nsl_table(void);
