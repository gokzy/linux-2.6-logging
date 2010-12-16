#include <linux/net_stack_logger.h>
#include <linux/ip.h>
#include <asm/atomic.h>
#include <net/ip.h>
#include <linux/compat.h>

static int nsl_init_module(void);
static void nsl_cleanup_module(void);
static int nsl_open(struct inode *, struct file *);
static int nsl_release(struct inode *, struct file *);
static ssize_t nsl_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t nsl_write(struct file *, const char __user *, size_t, loff_t *);
static long nsl_ioctl(struct file *, unsigned int, unsigned long);

static const struct file_operations nsl_fops = {
	.owner		= THIS_MODULE,
	.open		= nsl_open,
	.release	= nsl_release,
	.read		= nsl_read,
	.write		= nsl_write,
	.unlocked_ioctl	= nsl_ioctl,
};

module_init(nsl_init_module);
module_exit(nsl_cleanup_module);

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

static int __init nsl_init_module(void)
{
	if (register_chrdev(NSL_MAJOR, NSL_DEV_NAME, &nsl_fops)) {
		printk(KERN_ERR "nsl: unable to get major\n");
		return -EIO;
	}

	return 0;
}

static void nsl_cleanup_module(void)
{
	unregister_chrdev(NSL_MAJOR, NSL_DEV_NAME);
}

static int nsl_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);

	return 0;
}

static int nsl_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);

	return 0;
}

static ssize_t nsl_read(struct file *file, char __user *buf, size_t len, 
			loff_t *off)
{
	return -EINVAL;
}

static ssize_t nsl_write(struct file *file, const char __user *buf, 
			 size_t len, loff_t *off)
{
	return -EINVAL;
}

static long nsl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0, size;
	char __user *argp = compat_ptr(arg);

	switch (cmd) {
	case NSL_GET_INDEX:
		ret = atomic_read(&atomic_index);
		break;
	case NSL_GET_TABLE:
		get_user(size, argp);
		if ((ret = copy_to_user(argp, nsl_table, size)))
			return -EFAULT;
		break;
	default:
		return -EINVAL;
	}

	return ret;
}

