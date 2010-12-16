#include <linux/net_stack_logger.h>
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
	int ret = 0;
	unsigned int size;
	char __user *argp = compat_ptr(arg);

	switch (cmd) {
	case NSL_GET_INDEX:
		ret = atomic_read(&atomic_index);
		break;
	case NSL_GET_TABLE:
		if ((ret = copy_from_user(&size, argp, sizeof(unsigned int))))
			return -EFAULT;
		if ((ret = copy_to_user(argp, nsl_table, size)))
			return -EFAULT;
		break;
	default:
		return -EINVAL;
	}

	return ret;
}
