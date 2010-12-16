#include <linux/net_stack_logger.h>
#include <linux/ip.h>
#include <asm/atomic.h>
#include <net/ip.h>

// protocol 
#define IPv4 0x8
#define TCP  0x6
#define UDP  0x11

#define MAC_HEADER_LEN 14


static atomic_t atomic_index = ATOMIC_INIT(0);
static struct net_stack_log nsl_table[1000];


void logging_net_stack(unsigned int func, int cpu, struct sk_buff *skb)
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

	index = atomic_read(&atomic_index);

//	if(index < 1000 && skb->protocol == IPv4 && (ip->protocol == TCP || ip->protocol == UDP)){
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

		atomic_inc(&atomic_index);
	}

}


void debug_print_nsl_table(void)
{
  int i=0;
  int index = atomic_read(&atomic_index);

  printk(KERN_INFO "[udp_rcv %d] print_log\n",index);

  for(i=0; i<index; i++){
    printk(KERN_INFO 
	   "tb[%d] func:%u cpu:%d eth_proto:%x ip_proto:%x saddr:%x daddr:%x sport:%x dport:%x time:%llx\n"
	   ,i, nsl_table[i].func, nsl_table[i].cpu, nsl_table[i].eth_protocol, nsl_table[i].ip_protocol, 
	   nsl_table[i].ip_saddr, nsl_table[i].ip_daddr, nsl_table[i].tp_sport, nsl_table[i].tp_dport, nsl_table[i].time);
  }

}
