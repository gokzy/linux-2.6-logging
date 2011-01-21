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

#define NSL_ENABLE_NETWORK
//#define NSL_ENABLE_PROCESS

#define NSL_DEV_NAME "nsl"
#define NSL_MAJOR 261

#define NSL_GET_INDEX _IOWR(NSL_MAJOR, 1, unsigned long)
#define NSL_ENABLE _IO(NSL_MAJOR, 2)
#define NSL_DISABLE _IO(NSL_MAJOR, 3)

#define NSL_LOG_SIZE 1048576
#define NSL_MAX_CPU 8
#define NSL_TABLE_SIZE \
	(((sizeof(struct nsl_entry) * NSL_LOG_SIZE * NSL_MAX_CPU) & PAGE_MASK) \
	 + PAGE_SIZE)

#define NSL_POLL	16
#define NSL_NETIF_RECEIVE_SKB	1
#define NSL_BACKLOG_ENQUEUED	2
#define NSL_BACKLOG_DROPPED	15
#define NSL___NETIF_RECEIVE_SKB	3
#define NSL_IP_RCV		4
#define NSL_SK_DATA_READY	5
#define NSL_SKB_DEQUEUE		6
#define NSL_SKB_COPY		7

#define NSL_TCP_FOUND_FIN_OK 8
#define NSL_TCP_BEFORE_CLEANUP_RBUF 9
#define NSL_TCP_AFTER_CLEANUP_RBUF 10
#define NSL_TCP_OUT 11
#define NSL_TCP_RECV_URG 12

#define NSL_BEGIN_INET_RECVMSG 13
#define NSL_END_INET_RECVMSG 14

#define NSL_SCHEDULE 15

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
	uint64_t skb_id;
	uint64_t sock_id;
	uint64_t pktlen;
	uint32_t cnt;
	uint64_t len;
};

#ifdef __KERNEL__
extern int nsl_enable;
extern struct nsl_entry *nsl_table;
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

static inline void nsl_sock_setid(struct sock *sk)
{
	sk->id = nsl_gettime();
}

#ifdef NSL_ENABLE_NETWORK
#define nsl_log(func, skb) _nsl_log(func, skb, 0, 0)

static inline void _nsl_log(unsigned int func, struct sk_buff *skb,
						   uint32_t cnt, uint64_t len)
{
	int cpu = smp_processor_id();
	int index, i = cpu * NSL_LOG_SIZE;

	if (nsl_enable &&
	    (index = atomic_inc_return(&nsl_index[cpu])) < NSL_LOG_SIZE) {
		i += index;
		nsl_table[i].func = func;
		nsl_table[i].time = nsl_gettime();
		nsl_table[i].cnt = cnt;
		nsl_table[i].len = len;
		nsl_table[i].sock_id = 0;
		if (skb != NULL) {
			nsl_table[i].skb_id = skb->id;
			nsl_table[i].eth_protocol = skb->protocol;
			nsl_table[i].pktlen = skb->len;
			if (skb->protocol == htons(ETH_P_IP) && skb->head) {
				unsigned int mhdr = skb->mac_header + MAC_HEADER_LEN;
				struct iphdr *ip = (struct iphdr *)((char *)skb->head + mhdr);
				nsl_table[i].ip_protocol  = ip->protocol;
				nsl_table[i].ip_saddr = ip->saddr;
				nsl_table[i].ip_daddr = ip->daddr;
				nsl_table[i].ip_frag_off = ip->frag_off;
				switch (ip->protocol) {
				case IPPROTO_TCP: {
					struct tcphdr *tcp = (struct tcphdr *)
						((char *)skb->head + mhdr + (ip->ihl * 4));
					nsl_table[i].tp_sport = tcp->source;
					nsl_table[i].tp_dport = tcp->dest;
					break;
				}
				case IPPROTO_UDP: {
					struct udphdr *udp = (struct udphdr *)
						((char *)skb->head + mhdr + (ip->ihl * 4));
					nsl_table[i].tp_sport = udp->source;
					nsl_table[i].tp_dport = udp->dest;
					break;
				}
				default:
					nsl_table[i].tp_sport = 0;
					nsl_table[i].tp_dport = 0;
				}
			}else{
				nsl_table[i].ip_protocol = 0;
				nsl_table[i].ip_saddr = 0;
				nsl_table[i].ip_daddr = 0;
				nsl_table[i].ip_frag_off = 0;
				nsl_table[i].tp_sport = 0;
				nsl_table[i].tp_dport = 0;
			}
		}else
			nsl_table[i].eth_protocol = 0;
	}
}

static inline void nsl_log_sk(unsigned int func, struct sock *sk)
{
	struct inet_sock *inet = inet_sk(sk);
	int cpu = smp_processor_id();
	int index, i = cpu * NSL_LOG_SIZE;

	if (nsl_enable &&
	    (index = atomic_inc_return(&nsl_index[cpu])) < NSL_LOG_SIZE) {
		i += index;
		nsl_table[i].func = func;
		nsl_table[i].time = nsl_gettime();
		if (sk != NULL) {
			nsl_table[i].sock_id = sk->id;
			nsl_table[i].skb_id = sk->skb_id;
			nsl_table[i].cnt = sk->cnt;
			nsl_table[i].len = 0;
			nsl_table[i].pktlen = 0;
			nsl_table[i].eth_protocol = 0;
			nsl_table[i].ip_protocol  = 0;
			nsl_table[i].ip_saddr = inet->inet_saddr;
			nsl_table[i].ip_daddr = inet->inet_daddr;
			nsl_table[i].ip_frag_off = 0;
			nsl_table[i].tp_sport = inet->inet_sport;
			nsl_table[i].tp_dport = inet->inet_dport;
		}
	}
}
#else
#define nsl_log(func, skb)
#define _nsl_log(func, skb)
#define nsl_log_sk(func, sk)
#endif

#ifdef NSL_ENABLE_PROCESS
static inline void nsl_log_switch(unsigned int func, pid_t prev,
								  pid_t next, int switches)
{
	int cpu = smp_processor_id();
	int index, i = cpu * NSL_LOG_SIZE;

	if (nsl_enable &&
	    (index = atomic_inc_return(&nsl_index[cpu])) < NSL_LOG_SIZE) {
		i += index;
		nsl_table[i].func = func;
		nsl_table[i].time = nsl_gettime();
		nsl_table[i].sock_id = prev;
		nsl_table[i].skb_id = next;
		nsl_table[i].cnt = switches;
		nsl_table[i].len = 0;
		nsl_table[i].pktlen = 0;
		nsl_table[i].eth_protocol = 0;
		nsl_table[i].ip_protocol  = 0;
		nsl_table[i].ip_saddr = 0;
		nsl_table[i].ip_daddr = 0;
		nsl_table[i].ip_frag_off = 0;
		nsl_table[i].tp_sport = 0;
		nsl_table[i].tp_dport = 9;
	}
}
#else
#define nsl_log_switch(func, prev, next, swiches)
#endif
#endif
