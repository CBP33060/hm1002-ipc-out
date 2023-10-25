#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/mm.h>
#include <linux/seq_file.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/netlink.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define GLOBAL_EVENT_MAGIC 	'g'
#define GLOBAL_EVENT_GET_NET_STATUS 		_IOR(GLOBAL_EVENT_MAGIC, 0, int)
#define GLOBAL_EVENT_SET_NET_STATUS     	_IOW(GLOBAL_EVENT_MAGIC, 1, int)
#define GLOBAL_EVENT_GET_SLEEP_STATUS 		_IOR(GLOBAL_EVENT_MAGIC, 2, int)
#define GLOBAL_EVENT_SET_SLEEP_STATUS 		_IOW(GLOBAL_EVENT_MAGIC, 3, int)


#define GLOBAL_EVENT_MAJOR_NUM 	223
#define GLOGAL_EVENT_DEV_NAME 	"global_event_dev"

#define	GLOBAL_EVENT_NETLINK_PT			23
#define GLOBAL_EVENT_NETLINK_PORT_ID		50
#define GLOBAL_EVENT_NETLINK_GROUPS_ID		0x01

#define NETLINK_MSG_TYPE_BASE 		(GLOBAL_EVENT_NETLINK_PT)
#define NETLINK_MSG_NET_STATUS		(NETLINK_MSG_TYPE_BASE + 1)
#define NETLINK_MSG_SLEEP_STATUS	(NETLINK_MSG_TYPE_BASE + 2)

typedef struct _global_event_dev
{
	struct cdev cdev;
	struct class *cls;
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *proc_global_event;
#endif
	struct sock *nl_sock;
	int 	devno;
	int 	net_status;
	int 	sleep_status;
}t_global_event_dev;

t_global_event_dev g_global_event_dev;

#define MOD_SLEEP_NUM   4096
#define BUF_SIZE (MOD_SLEEP_NUM)
static struct page *start_page;

static int global_event_netlink_snd(uint16_t type, uint8_t *pbuf, uint16_t len)
{
	struct sk_buff *nl_skb;
	struct nlmsghdr *nlh;
	int ret = -1;

	nl_skb = nlmsg_new(len, GFP_ATOMIC);
	if (!nl_skb) 
	{
		printk(KERN_ERR"netlink_alloc_skb error\n");
		return ret;
	}

	nlh = nlmsg_put(nl_skb, 0, GLOBAL_EVENT_NETLINK_GROUPS_ID, type, len, 0);
	if (nlh == NULL) 
	{
		printk(KERN_ERR"nlmsg put error\n");
		nlmsg_free(nl_skb);
		return ret;
	}

	memcpy(nlmsg_data(nlh), pbuf, len);
	ret = nlmsg_multicast(g_global_event_dev.nl_sock, nl_skb, 
			0, GLOBAL_EVENT_NETLINK_GROUPS_ID, MSG_DONTWAIT);
	return ret;
}

static void global_event_netlink_rcv(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;
	void *data = NULL;

	printk(KERN_INFO"skb->len %u\n", skb->len);
	if (skb->len >= nlmsg_total_size(0))
	{
		nlh = nlmsg_hdr(skb);
		data = NLMSG_DATA(nlh);
		if (data) 
		{
			printk(KERN_ERR"kernel receive data : %s\n", (int8_t *)data);
		}
	}
}


struct netlink_kernel_cfg global_event_netlink_cfg = {
	.groups	= GLOBAL_EVENT_NETLINK_GROUPS_ID,
	.input = global_event_netlink_rcv,
};


static int global_event_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t global_event_read(struct file *file, char __user *ubuf, size_t size, loff_t *ppos)
{
	return 0;
}

static ssize_t global_event_write(struct file *file, const char __user *ubuf, size_t size, loff_t *ppos)
{
	return 0;
}

static long global_event_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int err;
	void __user *argp = (void __user *)arg;

	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_READ,argp,_IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_WRITE,argp,_IOC_SIZE(cmd));
	}

	if(err)
		return -EFAULT;


	switch(cmd)
	{
		case GLOBAL_EVENT_GET_NET_STATUS:
		{
			copy_to_user(argp, &g_global_event_dev.net_status, _IOC_SIZE(cmd));
		}
		break;
		case GLOBAL_EVENT_SET_NET_STATUS:
		{
			 copy_from_user(&g_global_event_dev.net_status, argp, _IOC_SIZE(cmd));
			 global_event_netlink_snd(NETLINK_MSG_NET_STATUS, 
			 	(uint8_t *)&g_global_event_dev.net_status, 
				sizeof(g_global_event_dev.net_status));
		}
		break;
		case GLOBAL_EVENT_GET_SLEEP_STATUS:
		{
			copy_to_user(argp, &g_global_event_dev.sleep_status, _IOC_SIZE(cmd));
		}
		break;
		case GLOBAL_EVENT_SET_SLEEP_STATUS:
		{
			 copy_from_user(&g_global_event_dev.sleep_status, argp, _IOC_SIZE(cmd));
			 global_event_netlink_snd(NETLINK_MSG_SLEEP_STATUS, 
			 	(uint8_t *)&g_global_event_dev.sleep_status, 
				sizeof(g_global_event_dev.sleep_status));
		}
		break;
	}

	return 0;
}

static int global_event_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long pfn_start = page_to_pfn(start_page) + vma->vm_pgoff;
	unsigned long virt_start = (unsigned long)page_address(start_page);
	unsigned long size = vma->vm_end - vma->vm_start;
	int ret = 0;

	printk("WCQ=====phy: 0x%lx, offset: 0x%lx, size: 0x%lx\n", pfn_start << PAGE_SHIFT, offset, size);

	ret = remap_pfn_range(vma, vma->vm_start, pfn_start, size, vma->vm_page_prot);
	if (ret)
		printk("WCQ=====%s: remap_pfn_range failed at [0x%lx  0x%lx]\n",
			__func__, vma->vm_start, vma->vm_end);
	else
		printk("WCQ=====%s: map 0x%lx to 0x%lx, size: 0x%lx\n", __func__, virt_start,
			vma->vm_start, size);

	return ret;
}

static int global_event_close(struct inode *indoe, struct file *file)  
{  
	return 0;  
}  

static const struct file_operations global_event_operations = {
	.owner = THIS_MODULE,
	.open = global_event_open,
	.read = global_event_read,
	.write = global_event_write,
	.unlocked_ioctl = global_event_unlocked_ioctl,
	.mmap = global_event_mmap,
	.release = global_event_close,
};

#ifdef CONFIG_PROC_FS
static int netstatus_show(struct seq_file *s, void *p)
{
        seq_printf(s, "%d\n", g_global_event_dev.net_status);
        return 0;
}

static int netstatus_open(struct inode *inode, struct file *file)
{
        return single_open(file, &netstatus_show, NULL);
}

static ssize_t netstatus_write(struct file *file,
        const char __user *buffer, size_t count, loff_t *pos)
{
	unsigned char kbuffer[5] = {0};
	if (count < 1)
		return -EINVAL;
	size_t length;
	length = count > sizeof(g_global_event_dev.net_status) ? 
	       sizeof(g_global_event_dev.net_status) : count; 
    
	if (copy_from_user(kbuffer, buffer, length))
                return -EFAULT;
	
	g_global_event_dev.net_status = (kbuffer[0] - 0x30);
        return count;
}

static const struct file_operations netstatus_proc_ops = {
	.open		= netstatus_open,
	.read		= seq_read,
	.write     = netstatus_write,
	.release	= single_release,
};
#endif

static int __init global_event_init(void)
{
	printk(KERN_INFO"global_event_init enter\n");
	int ret = -1;

	start_page = alloc_pages(GFP_KERNEL, get_order(BUF_SIZE));
	if (!start_page) {
		ret = -ENOMEM;
		goto err_register_chrdev_region;
	}

	g_global_event_dev.devno = MKDEV(GLOBAL_EVENT_MAJOR_NUM, 0);

	ret = register_chrdev_region(g_global_event_dev.devno, 0, GLOGAL_EVENT_DEV_NAME);
	if(ret < 0)
	{
		ret = alloc_chrdev_region(&g_global_event_dev.devno, 0, 1, GLOGAL_EVENT_DEV_NAME);
		if(ret < 0)
		{
			printk(KERN_ERR"global_event_dev cdev num region failed\n");
			goto err_register_chrdev_region;
		}
	}

	g_global_event_dev.cls = class_create(THIS_MODULE, GLOGAL_EVENT_DEV_NAME);
	if(IS_ERR(g_global_event_dev.cls))
	{
		ret = PTR_ERR(g_global_event_dev.cls);
		printk(KERN_ERR"global_event_dev class create failed\n");
		goto err_class_create;
	}


	cdev_init(&g_global_event_dev.cdev,&global_event_operations);

	ret = cdev_add(&g_global_event_dev.cdev, g_global_event_dev.devno, 1);
	if (ret < 0)
	{
		printk(KERN_ERR"global_event_dev cdev add failed\n");
		goto err_cdev_add;
	}

	struct device *devices= 
		device_create(g_global_event_dev.cls, NULL, 
				g_global_event_dev.devno, NULL, GLOGAL_EVENT_DEV_NAME);
	if(IS_ERR(devices))
	{
		ret = PTR_ERR(devices);
		printk(KERN_ERR,"global_event_dev device create failed\n");
		goto err_device_create;    
	}

	g_global_event_dev.nl_sock = netlink_kernel_create(&init_net, 
			GLOBAL_EVENT_NETLINK_PT, &global_event_netlink_cfg);    
	if (g_global_event_dev.nl_sock == NULL) 
	{
		printk(KERN_ERR"netlink init error\n");
		goto err_device_create;    
	}

#ifdef CONFIG_PROC_FS
	g_global_event_dev.proc_global_event = 
		proc_mkdir("global-event", NULL);
	if (!g_global_event_dev.proc_global_event)
	{
		goto err_device_create;
	}
	
	struct proc_dir_entry *entry;
	entry = proc_create("netstatus", 0644, 
			g_global_event_dev.proc_global_event,
			&netstatus_proc_ops);
        if (!entry)
	{
		goto err_proc_global;
	}
#endif
	printk(KERN_INFO"global_event_init exit\n");

	return 0;
#ifdef CONFIG_PROC_FS
err_proc_global:
	remove_proc_entry("global-event", NULL);
#endif
err_device_create:
	device_destroy(g_global_event_dev.cls, g_global_event_dev.devno);    

err_cdev_add:
	cdev_del(&g_global_event_dev.cdev);

err_class_create:
	unregister_chrdev_region(g_global_event_dev.devno, 1);

err_register_chrdev_region:
	__free_pages(start_page, get_order(BUF_SIZE));
	return ret;
}

static void __exit  global_event_exit(void)
{
	printk(KERN_INFO"global_event_exit enter\n");
	__free_pages(start_page, get_order(BUF_SIZE));
#ifdef CONFIG_PROC_FS
	remove_proc_entry("netstatus", g_global_event_dev.proc_global_event);
	remove_proc_entry("global-event", NULL);
#endif
	netlink_kernel_release(g_global_event_dev.nl_sock);
	g_global_event_dev.nl_sock = NULL;
	device_destroy(g_global_event_dev.cls, g_global_event_dev.devno);
	class_destroy(g_global_event_dev.cls);
	cdev_del(&g_global_event_dev.cdev);
	unregister_chrdev_region(g_global_event_dev.devno, 1);

	printk(KERN_INFO"global_event_exit exit\n");
}

module_init(global_event_init);
module_exit(global_event_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sunyangyang@70mai.com");
