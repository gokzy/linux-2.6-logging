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

int nsl_enable = 0;
struct net_stack_log **nsl_table;
atomic_t nsl_index[NSL_MAX_CPU] = {ATOMIC_INIT(-1),};

static int __init nsl_init_module(void)
{
	int i;

	nsl_table = (struct netstack_log **)kmalloc(
		sizeof(struct net_stack_log *) * NSL_MAX_CPU, GFP_KERNEL);
	if (!nsl_table) {
		printk("nsl: kmalloc failed\n");
		return -ENOMEM;
	}
	for (i = 0; i < NSL_MAX_CPU; i++) {
		nsl_table[i] = (struct net_stack_log *)kmalloc(
			sizeof(struct net_stack_log) * NSL_LOG_SIZE,
			GFP_KERNEL);
		if (!nsl_table[i]) {
			printk("nsl: kmalloc failed\n");
			return -ENOMEM;
		}
	}
		
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
	return 0;
}

static int nsl_release(struct inode *inode, struct file *file)
{
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
	int i, ret = 0;
	char __user *argp = compat_ptr(arg);

	switch (cmd) {
	case NSL_GET_TABLE:
		if ((ret = copy_to_user(argp, nsl_table, sizeof(nsl_table)))) {
			printk("copy_to_user failed: %d\n", ret);
			return -EFAULT;
		}
		break;
	case NSL_ENABLE:
		for (i = 0; i < NSL_MAX_CPU; i++)
			atomic_set(&nsl_index[i], -1);
		nsl_enable = 1;
		break;
	case NSL_DISABLE:
		nsl_enable = 0;
		break;
	default:
		return -EINVAL;
	}

	return ret;
}
