#include <linux/net_stack_logger.h>

struct net_stack_log nsl_table[1000];
atomic_t atomic_index = ATOMIC_INIT(0);

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
