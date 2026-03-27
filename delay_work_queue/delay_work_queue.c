/**********************************************
 * Author: lewiyon@hotmail.com
 * File name: delay_work_queue.c
 * Description: char device driver with delayed workqueue + Binder-like
 *   handle/message protocol.
 *
 *   Protocol overview
 *   -----------------
 *   write(fd, &dwq_msg,   sizeof(dwq_msg))   -- send request
 *   read (fd, &dwq_reply, sizeof(dwq_reply)) -- block until reply arrives
 *
 *   msg.handle   selects which "service" handles the request (like a
 *                Binder flat_binder_object handle).
 *   msg.cmd      DWQ_CMD_PING / DWQ_CMD_WORK / DWQ_CMD_QUERY
 *   msg.data[]   optional payload
 *
 * Date: 2011-12-24 (extended with handle/message protocol)
 *********************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/ioctl.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/device.h>

#include "dwq_uapi.h"

/* ------------------------------------------------------------------ */
/* handle table                                                         */
/* ------------------------------------------------------------------ */

/* One entry per registered handle. */
struct dwq_handle_entry {
    int   registered;           /* 1 if this slot is in use         */
    const char *name;           /* human-readable service name       */
    /* work state */
    int   running;              /* 1 = delayed work in flight        */
    int   done;                 /* 1 = work finished, reply ready    */
};

#define NR_HANDLES  DWQ_HANDLE_MAX

/* ------------------------------------------------------------------ */
/* device state                                                         */
/* ------------------------------------------------------------------ */
struct dwq_dev {
    /* char device bookkeeping */
    struct cdev              cdev;
    dev_t                    devno;
    struct class            *cls;
    struct device           *dev;

    /* workqueue – one shared queue, one delayed_work slot            */
    struct workqueue_struct *wq;
    struct delayed_work      dwork;

    /* handle table */
    struct dwq_handle_entry  handles[NR_HANDLES];

    /* pending request (saved when write() is called)                 */
    struct dwq_msg           req;

    /* pending reply (filled by work handler, consumed by read())     */
    struct dwq_reply         reply;

    /* synchronisation */
    struct mutex             lock;
    wait_queue_head_t        read_wq;   /* read() sleeps here        */
    int                      reply_ready; /* wake condition           */
};

static struct dwq_dev g_dwq;

/* ------------------------------------------------------------------ */
/* helpers                                                              */
/* ------------------------------------------------------------------ */
static void fill_reply(struct dwq_dev *dev, __u32 handle, __u32 cmd,
                        __s32 result, const char *msg_str)
{
    memset(&dev->reply, 0, sizeof(dev->reply));
    dev->reply.handle   = handle;
    dev->reply.cmd      = cmd;
    dev->reply.result   = result;
    if (msg_str) {
        size_t n = strlen(msg_str);
        if (n >= DWQ_MAX_DATA)
            n = DWQ_MAX_DATA - 1;
        memcpy(dev->reply.data, msg_str, n);
        dev->reply.data_len = (__u32)n;
    }
}

/* ------------------------------------------------------------------ */
/* work handler (runs in workqueue context, not user-process context)  */
/* ------------------------------------------------------------------ */
static void delay_func(struct work_struct *work)
{
    struct delayed_work *dw = to_delayed_work(work);
    struct dwq_dev *dev     = container_of(dw, struct dwq_dev, dwork);
    int i;
    __u32 handle = dev->req.handle;

    printk(KERN_INFO "dwq: [handle=%u] delay_func start, data=[%.*s]\n",
           handle, (int)dev->req.data_len, dev->req.data);

    /* simulate work: 3 × 1s */
    for (i = 0; i < 3; i++) {
        printk(KERN_INFO "dwq: [handle=%u] step %d/3\n", handle, i + 1);
        msleep(1000);
    }

    /* build reply */
    mutex_lock(&dev->lock);
    fill_reply(dev, handle, DWQ_CMD_WORK, DWQ_RESULT_OK, "work complete");
    dev->handles[handle].running = 0;
    dev->handles[handle].done    = 1;
    dev->reply_ready = 1;
    mutex_unlock(&dev->lock);

    wake_up_interruptible(&dev->read_wq);
    printk(KERN_INFO "dwq: [handle=%u] delay_func done\n", handle);
}

/* ------------------------------------------------------------------ */
/* handle-command dispatch (called from write(), under lock)           */
/* ------------------------------------------------------------------ */
static int dispatch_cmd(struct dwq_dev *dev, const struct dwq_msg *msg)
{
    __u32 h = msg->handle;

    /* validate handle */
    if (h >= NR_HANDLES || !dev->handles[h].registered) {
        fill_reply(dev, h, msg->cmd, DWQ_RESULT_ENODEV, "unknown handle");
        dev->reply_ready = 1;
        wake_up_interruptible(&dev->read_wq);
        return -ENODEV;
    }

    switch (msg->cmd) {

    case DWQ_CMD_PING:
        /* immediate reply, no workqueue needed */
        printk(KERN_INFO "dwq: [handle=%u] PING\n", h);
        fill_reply(dev, h, DWQ_CMD_PING, DWQ_RESULT_OK, "pong");
        dev->reply_ready = 1;
        wake_up_interruptible(&dev->read_wq);
        break;

    case DWQ_CMD_WORK:
        if (dev->handles[h].running) {
            fill_reply(dev, h, DWQ_CMD_WORK, DWQ_RESULT_EBUSY, "handle busy");
            dev->reply_ready = 1;
            wake_up_interruptible(&dev->read_wq);
            return -EBUSY;
        }
        dev->handles[h].running = 1;
        dev->handles[h].done    = 0;
        /* save request so delay_func can read it */
        memcpy(&dev->req, msg, sizeof(*msg));
        queue_delayed_work(dev->wq, &dev->dwork, 0);
        printk(KERN_INFO "dwq: [handle=%u] WORK queued\n", h);
        /* reply will arrive when delay_func completes */
        break;

    case DWQ_CMD_QUERY: {
        /* synchronous status reply */
        char status[32];
        snprintf(status, sizeof(status), "running=%d done=%d",
                 dev->handles[h].running, dev->handles[h].done);
        fill_reply(dev, h, DWQ_CMD_QUERY, DWQ_RESULT_OK, status);
        dev->reply_ready = 1;
        wake_up_interruptible(&dev->read_wq);
        break;
    }

    default:
        fill_reply(dev, h, msg->cmd, DWQ_RESULT_EINVAL, "unknown cmd");
        dev->reply_ready = 1;
        wake_up_interruptible(&dev->read_wq);
        return -EINVAL;
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/* file operations                                                      */
/* ------------------------------------------------------------------ */
static int dwq_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &g_dwq;
    printk(KERN_INFO "dwq: opened by pid %d\n", current->pid);
    return 0;
}

static int dwq_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "dwq: closed by pid %d\n", current->pid);
    return 0;
}

/*
 * write() – userspace sends a struct dwq_msg
 * Exactly sizeof(struct dwq_msg) bytes must be written.
 */
static ssize_t dwq_write(struct file *filp, const char __user *buf,
                          size_t count, loff_t *ppos)
{
    struct dwq_dev *dev = filp->private_data;
    struct dwq_msg  msg;

    if (count != sizeof(msg)) {
        printk(KERN_ERR "dwq: write size %zu != expected %zu\n",
               count, sizeof(msg));
        return -EINVAL;
    }

    if (copy_from_user(&msg, buf, sizeof(msg)))
        return -EFAULT;

    mutex_lock(&dev->lock);
    dev->reply_ready = 0;
    dispatch_cmd(dev, &msg);
    mutex_unlock(&dev->lock);

    return (ssize_t)count;
}

/*
 * read() – userspace receives a struct dwq_reply
 * Blocks until reply_ready is set (by dispatch_cmd or delay_func).
 */
static ssize_t dwq_read(struct file *filp, char __user *buf,
                         size_t count, loff_t *ppos)
{
    struct dwq_dev *dev = filp->private_data;
    int ret;

    if (count < sizeof(struct dwq_reply))
        return -EINVAL;

    ret = wait_event_interruptible(dev->read_wq, dev->reply_ready);
    if (ret)
        return ret;

    if (copy_to_user(buf, &dev->reply, sizeof(dev->reply)))
        return -EFAULT;

    mutex_lock(&dev->lock);
    dev->reply_ready = 0;
    mutex_unlock(&dev->lock);

    return (ssize_t)sizeof(dev->reply);
}

/* ioctl: START / STOP / STATUS (kept for convenience) */
static long dwq_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct dwq_dev *dev = filp->private_data;
    int status;

    switch (cmd) {
    case DWQ_IOC_START: {
        unsigned long delay_jiffies = msecs_to_jiffies((unsigned long)arg);
        mutex_lock(&dev->lock);
        dev->handles[DWQ_HANDLE_DEFAULT].running = 1;
        dev->handles[DWQ_HANDLE_DEFAULT].done    = 0;
        dev->reply_ready = 0;
        /* use default handle, DWQ_CMD_WORK */
        memset(&dev->req, 0, sizeof(dev->req));
        dev->req.handle = DWQ_HANDLE_DEFAULT;
        dev->req.cmd    = DWQ_CMD_WORK;
        queue_delayed_work(dev->wq, &dev->dwork, delay_jiffies);
        mutex_unlock(&dev->lock);
        printk(KERN_INFO "dwq: ioctl START delay=%lu ms\n", (unsigned long)arg);
        break;
    }
    case DWQ_IOC_STOP:
        cancel_delayed_work_sync(&dev->dwork);
        mutex_lock(&dev->lock);
        dev->handles[DWQ_HANDLE_DEFAULT].running = 0;
        mutex_unlock(&dev->lock);
        printk(KERN_INFO "dwq: ioctl STOP\n");
        break;
    case DWQ_IOC_STATUS:
        mutex_lock(&dev->lock);
        status = dev->handles[DWQ_HANDLE_DEFAULT].running;
        mutex_unlock(&dev->lock);
        if (copy_to_user((int __user *)arg, &status, sizeof(status)))
            return -EFAULT;
        break;
    default:
        return -ENOTTY;
    }
    return 0;
}

static const struct file_operations dwq_fops = {
    .owner          = THIS_MODULE,
    .open           = dwq_open,
    .release        = dwq_release,
    .read           = dwq_read,
    .write          = dwq_write,
    .unlocked_ioctl = dwq_ioctl,
};

/* ------------------------------------------------------------------ */
/* module init / exit                                                   */
/* ------------------------------------------------------------------ */
static int __init dwq_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&g_dwq.devno, 0, 1, "dwq");
    if (ret < 0) {
        printk(KERN_ERR "dwq: alloc_chrdev_region failed: %d\n", ret);
        return ret;
    }

    cdev_init(&g_dwq.cdev, &dwq_fops);
    g_dwq.cdev.owner = THIS_MODULE;

    ret = cdev_add(&g_dwq.cdev, g_dwq.devno, 1);
    if (ret < 0) {
        printk(KERN_ERR "dwq: cdev_add failed: %d\n", ret);
        unregister_chrdev_region(g_dwq.devno, 1);
        return ret;
    }

    mutex_init(&g_dwq.lock);
    init_waitqueue_head(&g_dwq.read_wq);

    g_dwq.wq = create_workqueue("dwq_wq");
    if (!g_dwq.wq) {
        printk(KERN_ERR "dwq: create_workqueue failed\n");
        cdev_del(&g_dwq.cdev);
        unregister_chrdev_region(g_dwq.devno, 1);
        return -ENOMEM;
    }

    INIT_DELAYED_WORK(&g_dwq.dwork, delay_func);

    /* register handle 0 — the only built-in service */
    g_dwq.handles[DWQ_HANDLE_DEFAULT].registered = 1;
    g_dwq.handles[DWQ_HANDLE_DEFAULT].name       = "default-worker";

    /* auto-create /dev/dwq via udev (no manual mknod needed) */
    g_dwq.cls = class_create("dwq");
    if (IS_ERR(g_dwq.cls)) {
        ret = PTR_ERR(g_dwq.cls);
        printk(KERN_ERR "dwq: class_create failed: %d\n", ret);
        destroy_workqueue(g_dwq.wq);
        cdev_del(&g_dwq.cdev);
        unregister_chrdev_region(g_dwq.devno, 1);
        return ret;
    }

    g_dwq.dev = device_create(g_dwq.cls, NULL, g_dwq.devno, NULL, "dwq");
    if (IS_ERR(g_dwq.dev)) {
        ret = PTR_ERR(g_dwq.dev);
        printk(KERN_ERR "dwq: device_create failed: %d\n", ret);
        class_destroy(g_dwq.cls);
        destroy_workqueue(g_dwq.wq);
        cdev_del(&g_dwq.cdev);
        unregister_chrdev_region(g_dwq.devno, 1);
        return ret;
    }

    printk(KERN_INFO "dwq: loaded  major=%d  /dev/dwq created\n",
           MAJOR(g_dwq.devno));

    return 0;
}

static void __exit dwq_exit(void)
{
    cancel_delayed_work_sync(&g_dwq.dwork);
    flush_workqueue(g_dwq.wq);
    destroy_workqueue(g_dwq.wq);
    device_destroy(g_dwq.cls, g_dwq.devno);
    class_destroy(g_dwq.cls);
    cdev_del(&g_dwq.cdev);
    unregister_chrdev_region(g_dwq.devno, 1);
    printk(KERN_INFO "dwq: unloaded\n");
}

module_init(dwq_init);
module_exit(dwq_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lewiyon <lewiyon@hotmail.com>");
MODULE_DESCRIPTION("Char device driver – Binder-like handle/message protocol over workqueue");
