#include <linux/net_stack_logger.h>
#include <linux/compat.h>

static int nsl_init_module(void);
static void nsl_cleanup_module(void);
static int nsl_open(struct inode *, struct file *);
static int nsl_release(struct inode *, struct file *);
static int nsl_mmap(struct file *, struct vm_area_struct *);
static long nsl_ioctl(struct file *, unsigned int, unsigned long);

static const struct file_operations nsl_fops = {
	.owner		= THIS_MODULE,
	.open		= nsl_open,
	.release	= nsl_release,
	.mmap		= nsl_mmap,
	.unlocked_ioctl	= nsl_ioctl,
};

module_init(nsl_init_module);
module_exit(nsl_cleanup_module);

int nsl_enable = 0;
struct nsl_entry *nsl_table = NULL;
atomic_t nsl_index[NSL_MAX_CPU];

static int __init nsl_init_module(void)
{
	int i;
	
	if (register_chrdev(NSL_MAJOR, NSL_DEV_NAME, &nsl_fops)) {
		printk(KERN_ERR "nsl: unable to get major\n");
		return -EIO;
	}	
	nsl_table = (struct nsl_entry *)vmalloc(NSL_TABLE_SIZE);
	if (!nsl_table) {
		printk(KERN_ERR "nsl: unable to allocate memory\n");
		return -ENOMEM;
	}
	for (i = 0; i < NSL_TABLE_SIZE; i+= PAGE_SIZE) {
		SetPageReserved(
			vmalloc_to_page((void*)(((unsigned long)nsl_table) + i)));
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

static int nsl_mmap(struct file *file, struct vm_area_struct *vma)
{
	int ret;
	long length = vma->vm_end - vma->vm_start;
	unsigned long start = vma->vm_start;
	char *vmalloc_area_ptr = (char *)nsl_table;
	unsigned long pfn;

	/* check length - do not allow larger mappings than the number of
	   pages allocated */
	if (length > NSL_TABLE_SIZE) {
		printk(KERN_ERR "nsl: mmap too large\n");
		return -EIO;
	}

	/* loop over all pages, map it page individually */
	while (length > 0) {
		pfn = vmalloc_to_pfn(vmalloc_area_ptr);
		if ((ret = remap_pfn_range(vma, start, pfn, PAGE_SIZE,
								   PAGE_SHARED)) < 0) {
			printk(KERN_ERR "nsl: remap_pfn_range failed\n");
			return ret;
		}
		start += PAGE_SIZE;
		vmalloc_area_ptr += PAGE_SIZE;
		length -= PAGE_SIZE;
	}
	
	return 0;
}

static long nsl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case NSL_GET_INDEX: {
		int index;
		if (!nsl_enable)
			return -1;
		index = atomic_read(&nsl_index[arg]);
		return index < NSL_LOG_SIZE ? index : NSL_LOG_SIZE - 1;
		break;
	}
	case NSL_ENABLE: {
		int i;

		for (i = 0; i < NSL_MAX_CPU; i++)
			atomic_set(&nsl_index[i], -1);
		nsl_enable = 1;
		break;
	}
	case NSL_DISABLE:
		nsl_enable = 0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
