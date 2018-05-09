#include <linux/module.h>
#include <linux/kernel.h>

#include "../include/knvmveri.h"

// only for testing
#define TRACE_LEN 105

static int Major = 250;
static struct sock *nl_sk = NULL;
struct Metadata *trace;
int cur_idx;
dev_t dev_no,dev;

struct cdev *kernel_cdev;


static void send_to_user(int metadata_len)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    // the message if the length of metadata
    //char msg[sizeof(int)];
    //int msg_size = (msg) + 1;
    int msg_size = sizeof(int);
    int res;

    //pr_info("Creating skb.\n");
    skb = nlmsg_new(NLMSG_ALIGN(msg_size), GFP_KERNEL);
    if (!skb) {
        pr_err("Allocation failure.\n");
        return;
    }

    nlh = nlmsg_put(skb, 0, 1, NLMSG_DONE, msg_size + 1, 0);
    //strcpy(nlmsg_data(nlh), msg);
    memcpy(nlmsg_data(nlh), &metadata_len, sizeof(int));

    //pr_info("Sending skb.\n");
    res = nlmsg_multicast(nl_sk, skb, 0, MYMGRP, GFP_KERNEL);
    if (res < 0)
        pr_info("nlmsg_multicast() error: %d\n", res);
    //else
    //    pr_info("Success.\n");
}


ssize_t read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
    unsigned long ret;
    int size;
    //printk(KERN_INFO "@ Inside read \n");
    if (cur_idx > TRACE_LEN)
        return 0;
    size = MIN(BUFFER_LEN, TRACE_LEN - cur_idx);
    printk(KERN_INFO "@ count=%lu\n", count);

    memcpy(nvmveri_dev.data, trace + cur_idx, size * sizeof(struct Metadata));
    cur_idx += size;

    ret = copy_to_user(buff, nvmveri_dev.data, count);
    
	return ret;
}


int open(struct inode *inode, struct file *filp)
{
    //printk(KERN_INFO "@ Inside open \n");
    if(down_interruptible(&nvmveri_dev.sem)) { //lock
        //printk(KERN_INFO "@ could not hold semaphore");
        return -1;
    }
    return 0;
}

int release(struct inode *inode, struct file *filp)
{
  //printk(KERN_INFO "@ Inside close \n");
  //printk(KERN_INFO "@ Releasing semaphore");
  up(&nvmveri_dev.sem);  //unlock
  return 0;
}

struct file_operations fops = {
    read:  read,
    //write:  write,
    open:   open,
    release: release
};


static int __init knvmeri_init(void)
{
    int i, ret;

    kernel_cdev = cdev_alloc();
    kernel_cdev->ops = &fops;
    kernel_cdev->owner = THIS_MODULE;
    printk ("@ Inside init module\n");
    ret = alloc_chrdev_region( &dev_no , 0, 1, "chr_arr_dev");
    if (ret < 0) {
        printk("@ Major number allocation is failed\n");
        return ret;
    }

    dev = MKDEV(Major,0);
    sema_init(&nvmveri_dev.sem,1);
    ret = cdev_add( kernel_cdev,dev,1);
    if(ret < 0 ) {
        printk(KERN_INFO "@ Unable to allocate cdev");
        return ret;
    }

    nl_sk = netlink_kernel_create(&init_net, MYPROTO, NULL);
    if (!nl_sk) {
        printk("@ Error creating socket.\n");
        return -10;
    }

    trace = (struct Metadata*) kmalloc(sizeof(struct Metadata) * TRACE_LEN, GFP_KERNEL);
    for (i = 0; i < TRACE_LEN; ++i) {
        trace[i].type = _ASSIGN;
        trace[i].assign.addr = (void*)(0x100 + i * 4);
        trace[i].assign.size = 4;
    }

    cur_idx = 0;
    send_to_user(TRACE_LEN);

    return 0;
}

static void __exit knvmeri_exit(void)
{
    netlink_kernel_release(nl_sk);
    kfree(trace);
    printk("@ Exiting hello module.\n");
}

module_init(knvmeri_init);
module_exit(knvmeri_exit);

MODULE_LICENSE("GPL");