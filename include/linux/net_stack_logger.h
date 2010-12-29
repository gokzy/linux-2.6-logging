#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/ioctl.h>
#include <asm/byteorder.h>

#ifdef __KERNEL__
#include <linux/hpet.h>
#include <linux/io.h>
#include <linux/skbuff.h>
#include <asm/hpet.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <asm/atomic.h>
#include <net/ip.h>
#endif

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

#define NSL_TCP_1 12
#define NSL_TCP_2 13
#define NSL_TCP_3 14
#define NSL_TCP_4 15

#define MAC_HEADER_LEN 14

struct nsl_entry {
	uint32_t func;
	uint16_t eth_protocol;
	uint8_t  ip_protocol;
	uint32_t ip_saddr;
	uint32_t ip_daddr;
	uint16_t ip_frag_off;
	uint16_t tp_sport;
	uint16_t tp_dport;
	uint64_t time;
	uint64_t id;
};

#ifdef __KERNEL__
extern int nsl_enable;
extern struct nsl_entry nsl_table[NSL_MAX_CPU][NSL_LOG_SIZE];
extern atomic_t nsl_index[];
extern void __iomem *hpet_virt_address;

static inline __u64 nsl_gettime(void)
{
	return readq(hpet_virt_address + HPET_COUNTER);
}

static inline void nsl_setid(struct sk_buff *skb)
{
	skb->id = nsl_gettime();
}

static inline void nsl_log(unsigned int func, struct sk_buff *skb)
{
	int cpu = smp_processor_id();
	int index;

	if (nsl_enable &&
	    (index = atomic_inc_return(&nsl_index[cpu])) < NSL_LOG_SIZE) {
		nsl_table[cpu][index].func = func;
		nsl_table[cpu][index].time = nsl_gettime();
		if (skb != NULL) {
			nsl_table[cpu][index].id = skb->id;
			nsl_table[cpu][index].eth_protocol = skb->protocol;
			if (skb->protocol == htons(ETH_P_IP) && skb->head) {
				unsigned int mhdr = skb->mac_header + MAC_HEADER_LEN;
				struct iphdr *ip = (struct iphdr *)((char *)skb->head + mhdr);
				nsl_table[cpu][index].ip_protocol  = ip->protocol;
				nsl_table[cpu][index].ip_saddr = ip->saddr;
				nsl_table[cpu][index].ip_daddr = ip->daddr;
				nsl_table[cpu][index].ip_frag_off = ip->frag_off;
				switch (ip->protocol) {
				case IPPROTO_TCP: {
					struct tcphdr *tcp = (struct tcphdr *)
						((char *)skb->head + mhdr + (ip->ihl * 4));
					nsl_table[cpu][index].tp_sport = tcp->source;
					nsl_table[cpu][index].tp_dport = tcp->dest;
					break;
				}
				case IPPROTO_UDP: {
					struct udphdr *udp = (struct udphdr *)
						((char *)skb->head + mhdr + (ip->ihl * 4));
					nsl_table[cpu][index].tp_sport = udp->source;
					nsl_table[cpu][index].tp_dport = udp->dest;
					break;
				}
				default:
					nsl_table[cpu][index].tp_sport = 0;
					nsl_table[cpu][index].tp_dport = 0;
				}
			}else{
				nsl_table[cpu][index].ip_protocol = 0;
				nsl_table[cpu][index].ip_saddr = 0;
				nsl_table[cpu][index].ip_daddr = 0;
				nsl_table[cpu][index].ip_frag_off = 0;
				nsl_table[cpu][index].tp_sport = 0;
				nsl_table[cpu][index].tp_dport = 0;
			}
		}else
			nsl_table[cpu][index].eth_protocol = 0;
	}
}
#endif
