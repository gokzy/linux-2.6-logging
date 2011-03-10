#ifndef _NET_STACK_LOGGER
#define _NET_STACK_LOGGER

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
#define NSL_ENABLE_IRQ

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


#define NSL_START_LOGGING           1
#define NSL_POLLING                 2
#define NSL_GRO_PROCESS             3
#define NSL_RPS_ENQUEUE_BACKLOG     4
#define NSL_RPS_DEQUEUE_BACKLOG     5

#define NSL_IP_RCV                  6
#define NSL_ENQUEUE_FRAGMENTS       7

#define NSL_TCP_V4_RCV              8
#define NSL_TCP_ENQUEUE_BACKLOG     9
#define NSL_TCP_V4_DO_RCV           10
#define NSL_TCP_DEQUEUE_BACKLOG     11
#define NSL_FASTPATH_DATA_READY     12
#define NSL_SLOWPATH_DATA_READY     13
#define NSL_OFOPATH_DATA_READY      14
#define NSL_SYSCALL_PROCESS_START   15
#define NSL_COPY_SKB_COMPLETE       16
#define NSL_DEQUEUE_RECEIVE_QUEUE   17

#define NSL_UDP_RCV 18
#define NSL_UDP_DATA_READY 19


#define NSL_TCP_FOUND_FIN_OK 30
#define NSL_TCP_BEFORE_CLEANUP_RBUF 31
#define NSL_TCP_AFTER_CLEANUP_RBUF 32
#define NSL_TCP_OUT 33
#define NSL_TCP_RECV_URG 34

#define NSL_BEGIN_INET_RECVMSG 50
#define NSL_AFTER_BACKLOG_PROCESS 51
#define NSL_END_INET_RECVMSG   52


#define NSL_SCHEDULE 100

#define NSL_START_IRQ 200
#define NSL_END_IRQ 201
#define NSL_START_IPI 202
#define NSL_END_IPI 203

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
	uint32_t receive_qlen;
	uint32_t backlog_qlen;
	uint32_t tcp_seq;
	uint32_t ip_id;
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
#define nsl_log(func, skb)                    __nsl_log(func, skb, 0, NULL)
#define nsl_cnt_log(func, skb, cnt)           __nsl_log(func, skb, cnt, NULL)
#define nsl_cnt_queue_log(func, skb, cnt, sk) __nsl_log(func, skb, cnt, sk)

static inline void __nsl_log(unsigned int func, struct sk_buff *skb,
			     uint32_t cnt, struct sock *sk)
{
	int cpu = smp_processor_id();
	int index, i = cpu * NSL_LOG_SIZE;

	if (nsl_enable &&
	    (index = atomic_inc_return(&nsl_index[cpu])) < NSL_LOG_SIZE) {

		i += index;
		nsl_table[i].func = func;
		nsl_table[i].time = nsl_gettime();
		nsl_table[i].cnt = cnt;

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
				nsl_table[i].ip_id       = ip->id;

				switch (ip->protocol) {
				case IPPROTO_TCP: {
					struct tcphdr *tcp = (struct tcphdr *)
						((char *)skb->head + mhdr + (ip->ihl * 4));
					nsl_table[i].tp_sport = tcp->source;
					nsl_table[i].tp_dport = tcp->dest;
					nsl_table[i].tcp_seq = tcp->seq;
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
				
				if(sk != NULL){
					nsl_table[i].sock_id = sk->id;
					nsl_table[i].receive_qlen = sk->sk_receive_queue.qlen;
					nsl_table[i].backlog_qlen = sk->sk_backlog.len;
				}else{
					nsl_table[i].sock_id = 0;
					nsl_table[i].receive_qlen = 0;
					nsl_table[i].backlog_qlen = 0;
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


static inline void nsl_log_sk(unsigned int func, struct sock *sk, uint32_t cnt)
{
	struct inet_sock *inet = inet_sk(sk);
	int cpu = smp_processor_id();
	int index, i = cpu * NSL_LOG_SIZE;

	if (nsl_enable &&
	    (index = atomic_inc_return(&nsl_index[cpu])) < NSL_LOG_SIZE) {
		i += index;
		nsl_table[i].func = func;
		nsl_table[i].time = nsl_gettime();
		nsl_table[i].cnt  = cnt;

		if (sk != NULL) {
			nsl_table[i].sock_id = sk->id;
			nsl_table[i].pktlen = 0;
			nsl_table[i].eth_protocol = 0;
			nsl_table[i].ip_protocol  = 0;
			nsl_table[i].ip_saddr = inet->inet_saddr;
			nsl_table[i].ip_daddr = inet->inet_daddr;
			nsl_table[i].ip_frag_off = 0;
			nsl_table[i].tp_sport = inet->inet_sport;
			nsl_table[i].tp_dport = inet->inet_dport;
			nsl_table[i].receive_qlen = sk->sk_receive_queue.qlen;
			nsl_table[i].backlog_qlen = sk->sk_backlog.len;
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
		nsl_table[i].tp_dport = 0;
	}
}
#else
#define nsl_log_switch(func, prev, next, swiches)
#endif
#endif

#ifdef NSL_ENABLE_IRQ
static inline void nsl_log_irq(unsigned int func, unsigned irq)
{
	int cpu = smp_processor_id();
	int index, i = cpu * NSL_LOG_SIZE;

	if (nsl_enable &&
	    (index = atomic_inc_return(&nsl_index[cpu])) < NSL_LOG_SIZE) {
		i += index;
		nsl_table[i].func = func;
		nsl_table[i].time = nsl_gettime();
		nsl_table[i].sock_id = irq;
		nsl_table[i].skb_id = 0;
		nsl_table[i].cnt = 0;
		nsl_table[i].pktlen = 0;
		nsl_table[i].eth_protocol = 0;
		nsl_table[i].ip_protocol  = 0;
		nsl_table[i].ip_saddr = 0;
		nsl_table[i].ip_daddr = 0;
		nsl_table[i].ip_frag_off = 0;
		nsl_table[i].tp_sport = 0;
		nsl_table[i].tp_dport = 0;
	}
}
#else
#define nsl_log_irq(func, irq)
#endif

#endif //NET_STACK_LOGGER
