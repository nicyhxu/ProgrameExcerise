/**********************************************
 * Author: lewiyon@hotmail.com
 * File name: delay_work_queue.c
 * Description: char device driver – Binder-like IPC with service
 *   registration and per-fd transaction routing.
 *
 *   Architecture
 *   ────────────
 *   handle 表  (类比 binder_node / binder_ref)
 *     handle=0  : 内置 workqueue 服务（向后兼容）
 *     handle=1~7: 用户空间 server 通过 ioctl(REGISTER) 注册
 *                 handles[N].server_ctx 指向 server 的 fd_ctx
 *                 （类比 binder_node.proc 指向 server binder_proc）
 *
 *   per-fd context  dwq_fd_ctx  (类比 binder_thread)
 *     client fd: done_list    — 存放已完成的回复，read() 在此等待
 *     server fd: pending_list — 存放待处理的请求，read() 在此等待
 *                is_server=1 区分两种角色
 *
 *   事务流  (类比 binder_transaction)
 *     client write(handle=N, cmd=WORK)
 *       → 查 handles[N].server_ctx
 *       → 若有 server: tx 挂入 server_ctx->pending_list, wake server
 *       → 若无 server 且 N==0: 走 workqueue
 *       → client read() 等 done_list
 *
 *     server read()  拿到 dwq_msg（含 tx_id）
 *     server 处理完  write(cmd=REPLY, tx_id=原值)
 *       → 驱动找到 tx->caller, 放入 caller->done_list, wake client
 *
 * Date: 2011-12-24 (service registration + routing)
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
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/ioctl.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/atomic.h>
#include <linux/idr.h>

#include "dwq_uapi.h"

/* ------------------------------------------------------------------ */
/* forward declaration                                                  */
/* ------------------------------------------------------------------ */
struct dwq_fd_ctx;

/* ------------------------------------------------------------------ */
/* transaction  (类比 binder_transaction)                              */
/* ------------------------------------------------------------------ */
struct dwq_transaction {
    struct list_head   node;
    struct dwq_msg     msg;          /* 原始请求（含 tx_id）            */
    struct dwq_reply   reply;        /* 回复内容                        */
    struct dwq_fd_ctx *caller;       /* 发起方 fd_ctx，回复时精确唤醒   */
};

/* ------------------------------------------------------------------ */
/* per-fd context  (类比 binder_thread)                                */
/* ------------------------------------------------------------------ */
struct dwq_fd_ctx {
    /* 通用 */
    wait_queue_head_t  wait;
    atomic_t           tx_id_counter; /* 每 write() 递增，填入 tx_id   */

    /* client 侧：已完成回复队列，read() 等这里 */
    struct list_head   done_list;
    spinlock_t         done_lock;

    /* server 侧：待处理请求队列，read() 等这里 */
    struct list_head   pending_list;
    spinlock_t         pending_lock;

    /* 角色标记 */
    int                is_server;
    __u32              server_handle;  /* 注册的 handle 编号            */
};

/* ------------------------------------------------------------------ */
/* handle 表项  (类比 binder_node)                                     */
/* ------------------------------------------------------------------ */
struct dwq_handle_entry {
    int                registered;
    char               name[32];
    struct dwq_fd_ctx *server_ctx;  /* 指向 server 的 fd_ctx           */
    struct mutex       lock;        /* 保护注册/注销的并发安全          */
};

#define NR_HANDLES  DWQ_HANDLE_MAX

/* ------------------------------------------------------------------ */
/* shared memory layout (mmap)                                         */
/* ------------------------------------------------------------------ */
#define DWQ_SHM_SIZE  PAGE_SIZE

struct dwq_shm {
    __u32  seq;
    __u32  state;
    char   data[DWQ_MAX_DATA];
    char   reply_data[DWQ_MAX_DATA];
};

/* ------------------------------------------------------------------ */
/* global device                                                        */
/* ------------------------------------------------------------------ */
struct dwq_dev {
    struct cdev              cdev;
    dev_t                    devno;
    struct class            *cls;
    struct device            *dev;

    /* handle 表 */
    struct dwq_handle_entry  handles[NR_HANDLES];

    /* handle=0 的内置 workqueue（向后兼容）*/
    struct workqueue_struct *wq;
    struct delayed_work      dwork;
    spinlock_t               wq_lock;
    struct list_head         wq_pending; /* workqueue 的待处理队列     */

    /* shared memory */
    struct page             *shm_page;
    struct dwq_shm          *shm;

    struct mutex             lock;
};

static struct dwq_dev g_dwq;

/* ------------------------------------------------------------------ */
/* 内置 workqueue handler（handle=0，向后兼容）                        */
/* ------------------------------------------------------------------ */
static void delay_func(struct work_struct *work)
{
    struct delayed_work    *dw  = to_delayed_work(work);
    struct dwq_dev         *dev = container_of(dw, struct dwq_dev, dwork);
    struct dwq_transaction *tx;
    unsigned long           flags;
    int i;

    spin_lock_irqsave(&dev->wq_lock, flags);
    if (list_empty(&dev->wq_pending)) {
        spin_unlock_irqrestore(&dev->wq_lock, flags);
        return;
    }
    tx = list_first_entry(&dev->wq_pending, struct dwq_transaction, node);
    list_del(&tx->node);
    spin_unlock_irqrestore(&dev->wq_lock, flags);

    printk(KERN_INFO "dwq: [wq] handle=%u data=[%.*s]\n",
           tx->msg.handle, (int)tx->msg.data_len, tx->msg.data);

    for (i = 0; i < 3; i++) {
        printk(KERN_INFO "dwq: [wq] step %d/3\n", i + 1);
        msleep(1000);
    }

    /* fill reply */
    memset(&tx->reply, 0, sizeof(tx->reply));
    tx->reply.handle   = tx->msg.handle;
    tx->reply.cmd      = tx->msg.cmd;
    tx->reply.tx_id    = tx->msg.tx_id;
    tx->reply.result   = DWQ_RESULT_OK;
    snprintf(tx->reply.data, DWQ_MAX_DATA,
             "wq done tx_id=%u", tx->msg.tx_id);
    tx->reply.data_len = strlen(tx->reply.data);

    /* 精确唤醒 caller */
    spin_lock_irqsave(&tx->caller->done_lock, flags);
    list_add_tail(&tx->node, &tx->caller->done_list);
    spin_unlock_irqrestore(&tx->caller->done_lock, flags);
    wake_up_interruptible(&tx->caller->wait);

    printk(KERN_INFO "dwq: [wq] done, woke caller\n");

    /* 还有待处理的则继续 */
    spin_lock_irqsave(&dev->wq_lock, flags);
    if (!list_empty(&dev->wq_pending))
        queue_delayed_work(dev->wq, &dev->dwork, 0);
    spin_unlock_irqrestore(&dev->wq_lock, flags);
}

/* ------------------------------------------------------------------ */
/* file operations                                                      */
/* ------------------------------------------------------------------ */
static int dwq_open(struct inode *inode, struct file *filp)
{
    struct dwq_fd_ctx *ctx;

    ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
    if (!ctx)
        return -ENOMEM;

    init_waitqueue_head(&ctx->wait);
    atomic_set(&ctx->tx_id_counter, 0);
    INIT_LIST_HEAD(&ctx->done_list);
    spin_lock_init(&ctx->done_lock);
    INIT_LIST_HEAD(&ctx->pending_list);
    spin_lock_init(&ctx->pending_lock);
    ctx->is_server = 0;

    filp->private_data = ctx;
    printk(KERN_INFO "dwq: open pid=%d\n", current->pid);
    return 0;
}

static int dwq_release(struct inode *inode, struct file *filp)
{
    struct dwq_fd_ctx      *ctx = filp->private_data;
    struct dwq_transaction *tx, *tmp;
    unsigned long           flags;

    /* server 关闭时自动注销 handle，防止悬空指针 */
    if (ctx->is_server) {
        __u32 h = ctx->server_handle;
        if (h < NR_HANDLES) {
            mutex_lock(&g_dwq.handles[h].lock);
            g_dwq.handles[h].server_ctx  = NULL;
            g_dwq.handles[h].registered  = 0;
            mutex_unlock(&g_dwq.handles[h].lock);
            printk(KERN_INFO "dwq: handle %u auto-unregistered on close\n", h);
        }
    }

    /* 释放未消费的回复 */
    spin_lock_irqsave(&ctx->done_lock, flags);
    list_for_each_entry_safe(tx, tmp, &ctx->done_list, node) {
        list_del(&tx->node);
        kfree(tx);
    }
    spin_unlock_irqrestore(&ctx->done_lock, flags);

    /* 释放未处理的请求（server 侧） */
    spin_lock_irqsave(&ctx->pending_lock, flags);
    list_for_each_entry_safe(tx, tmp, &ctx->pending_list, node) {
        list_del(&tx->node);
        /* 通知 caller 服务不可用 */
        tx->reply.handle  = tx->msg.handle;
        tx->reply.cmd     = tx->msg.cmd;
        tx->reply.tx_id   = tx->msg.tx_id;
        tx->reply.result  = DWQ_RESULT_ENODEV;
        snprintf(tx->reply.data, DWQ_MAX_DATA, "server closed");
        tx->reply.data_len = strlen(tx->reply.data);

        spin_lock(&tx->caller->done_lock);
        list_add_tail(&tx->node, &tx->caller->done_list);
        spin_unlock(&tx->caller->done_lock);
        wake_up_interruptible(&tx->caller->wait);
    }
    spin_unlock_irqrestore(&ctx->pending_lock, flags);

    kfree(ctx);
    printk(KERN_INFO "dwq: close pid=%d\n", current->pid);
    return 0;
}

/*
 * write() 双重职责：
 *   client: 发送请求（cmd != DWQ_CMD_REPLY）
 *   server: 发送回复（cmd == DWQ_CMD_REPLY）
 * 类比 Binder BC_TRANSACTION / BC_REPLY
 */
static ssize_t dwq_write(struct file *filp, const char __user *buf,
                          size_t count, loff_t *ppos)
{
    struct dwq_fd_ctx      *ctx = filp->private_data;
    struct dwq_dev         *dev = &g_dwq;
    struct dwq_msg          msg;
    struct dwq_transaction *tx;
    unsigned long           flags;

    if (count != sizeof(msg))
        return -EINVAL;
    if (copy_from_user(&msg, buf, sizeof(msg)))
        return -EFAULT;

    /* ---- server 回复路径 (BC_REPLY) ---- */
    if (msg.cmd == DWQ_CMD_REPLY) {
        if (!ctx->is_server)
            return -EPERM;

        /* 用 tx_id 在 server 的 pending_list 里找原始事务 */
        tx = NULL;
        spin_lock_irqsave(&ctx->pending_lock, flags);
        {
            struct dwq_transaction *t;
            list_for_each_entry(t, &ctx->pending_list, node) {
                if (t->msg.tx_id == msg.tx_id) {
                    tx = t;
                    list_del(&tx->node);
                    break;
                }
            }
        }
        spin_unlock_irqrestore(&ctx->pending_lock, flags);

        if (!tx) {
            printk(KERN_ERR "dwq: REPLY unknown tx_id=%u\n", msg.tx_id);
            return -EINVAL;
        }

        /* 填充回复，路由回 caller */
        tx->reply.handle   = msg.handle;
        tx->reply.cmd      = DWQ_CMD_REPLY;
        tx->reply.tx_id    = msg.tx_id;
        tx->reply.result   = DWQ_RESULT_OK;
        tx->reply.data_len = min(msg.data_len, (__u32)DWQ_MAX_DATA);
        memcpy(tx->reply.data, msg.data, tx->reply.data_len);

        spin_lock_irqsave(&tx->caller->done_lock, flags);
        list_add_tail(&tx->node, &tx->caller->done_list);
        spin_unlock_irqrestore(&tx->caller->done_lock, flags);
        wake_up_interruptible(&tx->caller->wait);

        printk(KERN_INFO "dwq: REPLY tx_id=%u → woke client\n", msg.tx_id);
        return (ssize_t)count;
    }

    /* ---- client 请求路径 (BC_TRANSACTION) ---- */
    if (msg.handle >= NR_HANDLES) {
        printk(KERN_ERR "dwq: invalid handle %u\n", msg.handle);
        return -EINVAL;
    }

    tx = kzalloc(sizeof(*tx), GFP_KERNEL);
    if (!tx)
        return -ENOMEM;

    /* 分配 tx_id */
    msg.tx_id = (__u32)atomic_inc_return(&ctx->tx_id_counter);
    memcpy(&tx->msg, &msg, sizeof(msg));
    tx->caller = ctx;

    /* 查 handle 表 — 类比 binder_get_ref → binder_node → binder_proc */
    mutex_lock(&dev->handles[msg.handle].lock);

    if (dev->handles[msg.handle].registered &&
        dev->handles[msg.handle].server_ctx != NULL) {
        /* 有用户空间 server 注册 → 路由到 server 的 pending_list */
        struct dwq_fd_ctx *srv = dev->handles[msg.handle].server_ctx;
        mutex_unlock(&dev->handles[msg.handle].lock);

        spin_lock_irqsave(&srv->pending_lock, flags);
        list_add_tail(&tx->node, &srv->pending_list);
        spin_unlock_irqrestore(&srv->pending_lock, flags);
        wake_up_interruptible(&srv->wait);

        printk(KERN_INFO "dwq: TX handle=%u cmd=%u tx_id=%u → server\n",
               msg.handle, msg.cmd, msg.tx_id);

    } else if (msg.handle == DWQ_HANDLE_DEFAULT) {
        /* handle=0 且无用户 server → 内置 workqueue */
        mutex_unlock(&dev->handles[msg.handle].lock);

        spin_lock_irqsave(&dev->wq_lock, flags);
        list_add_tail(&tx->node, &dev->wq_pending);
        queue_delayed_work(dev->wq, &dev->dwork, 0);
        spin_unlock_irqrestore(&dev->wq_lock, flags);

        printk(KERN_INFO "dwq: TX handle=0 tx_id=%u → workqueue\n",
               msg.tx_id);

    } else {
        mutex_unlock(&dev->handles[msg.handle].lock);
        kfree(tx);
        printk(KERN_ERR "dwq: handle %u not registered\n", msg.handle);
        return -ENODEV;
    }

    return (ssize_t)count;
}

/*
 * read() 双重职责：
 *   client (is_server=0): 等 done_list，返回 dwq_reply
 *   server (is_server=1): 等 pending_list，返回 dwq_msg
 * 类比 Binder binder_thread_read() 返回 BR_REPLY / BR_TRANSACTION
 */
static ssize_t dwq_read(struct file *filp, char __user *buf,
                         size_t count, loff_t *ppos)
{
    struct dwq_fd_ctx      *ctx = filp->private_data;
    struct dwq_transaction *tx  = NULL;
    unsigned long           flags;
    int ret;

    if (ctx->is_server) {
        /* server 等请求 */
        if (count < sizeof(struct dwq_msg))
            return -EINVAL;

        ret = wait_event_interruptible(ctx->wait,
                                       !list_empty(&ctx->pending_list));
        if (ret)
            return ret;

        spin_lock_irqsave(&ctx->pending_lock, flags);
        if (!list_empty(&ctx->pending_list))
            tx = list_first_entry(&ctx->pending_list,
                                  struct dwq_transaction, node);
            /* 注意：tx 留在 pending_list，等 server write(REPLY) 时按 tx_id 查找 */
            /* 所以这里不 list_del */
        spin_unlock_irqrestore(&ctx->pending_lock, flags);

        if (!tx)
            return -EAGAIN;

        if (copy_to_user(buf, &tx->msg, sizeof(tx->msg)))
            return -EFAULT;

        printk(KERN_INFO "dwq: server read tx_id=%u cmd=%u\n",
               tx->msg.tx_id, tx->msg.cmd);
        return (ssize_t)sizeof(struct dwq_msg);

    } else {
        /* client 等回复 */
        if (count < sizeof(struct dwq_reply))
            return -EINVAL;

        ret = wait_event_interruptible(ctx->wait,
                                       !list_empty(&ctx->done_list));
        if (ret)
            return ret;

        spin_lock_irqsave(&ctx->done_lock, flags);
        if (!list_empty(&ctx->done_list)) {
            tx = list_first_entry(&ctx->done_list,
                                  struct dwq_transaction, node);
            list_del(&tx->node);
        }
        spin_unlock_irqrestore(&ctx->done_lock, flags);

        if (!tx)
            return -EAGAIN;

        if (copy_to_user(buf, &tx->reply, sizeof(tx->reply))) {
            kfree(tx);
            return -EFAULT;
        }

        printk(KERN_INFO "dwq: client read reply tx_id=%u result=%d\n",
               tx->reply.tx_id, tx->reply.result);
        kfree(tx);
        return (ssize_t)sizeof(struct dwq_reply);
    }
}

static long dwq_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct dwq_fd_ctx *ctx = filp->private_data;
    struct dwq_dev    *dev = &g_dwq;

    switch (cmd) {

    case DWQ_IOC_REGISTER: {
        /* server 注册 handle — 类比 addService */
        struct dwq_reg_info info;
        __u32 h;

        if (copy_from_user(&info, (void __user *)arg, sizeof(info)))
            return -EFAULT;

        h = info.handle;
        if (h == DWQ_HANDLE_DEFAULT || h >= NR_HANDLES)
            return -EINVAL;  /* handle=0 保留给 workqueue */

        mutex_lock(&dev->handles[h].lock);
        if (dev->handles[h].server_ctx != NULL) {
            mutex_unlock(&dev->handles[h].lock);
            return -EBUSY;   /* 已有 server 注册 */
        }
        dev->handles[h].registered = 1;
        dev->handles[h].server_ctx = ctx;
        strncpy(dev->handles[h].name, info.name,
                sizeof(dev->handles[h].name) - 1);
        mutex_unlock(&dev->handles[h].lock);

        ctx->is_server     = 1;
        ctx->server_handle = h;

        printk(KERN_INFO "dwq: handle %u registered as \"%s\" pid=%d\n",
               h, info.name, current->pid);
        break;
    }

    case DWQ_IOC_UNREGISTER: {
        /* server 注销 — 类比 removeService */
        __u32 h;

        if (!ctx->is_server)
            return -EPERM;

        h = ctx->server_handle;
        mutex_lock(&dev->handles[h].lock);
        dev->handles[h].server_ctx = NULL;
        dev->handles[h].registered = 0;
        mutex_unlock(&dev->handles[h].lock);

        ctx->is_server = 0;
        printk(KERN_INFO "dwq: handle %u unregistered pid=%d\n",
               h, current->pid);
        break;
    }

    case DWQ_IOC_STATUS: {
        int depth;
        if (ctx->is_server) {
            unsigned long flags;
            int cnt = 0;
            struct dwq_transaction *t;
            spin_lock_irqsave(&ctx->pending_lock, flags);
            list_for_each_entry(t, &ctx->pending_list, node) cnt++;
            spin_unlock_irqrestore(&ctx->pending_lock, flags);
            depth = cnt;
        } else {
            depth = !list_empty(&dev->wq_pending);
        }
        if (copy_to_user((int __user *)arg, &depth, sizeof(depth)))
            return -EFAULT;
        break;
    }

    case DWQ_IOC_STOP:
        cancel_delayed_work_sync(&dev->dwork);
        break;

    default:
        return -ENOTTY;
    }
    return 0;
}

static int dwq_mmap(struct file *filp, struct vm_area_struct *vma)
{
    struct dwq_dev *dev = &g_dwq;
    unsigned long pfn;
    int ret;

    if ((vma->vm_end - vma->vm_start) > DWQ_SHM_SIZE)
        return -EINVAL;

    pfn = page_to_pfn(dev->shm_page);
    vma->vm_flags |= VM_SHARED;
    ret = remap_pfn_range(vma, vma->vm_start, pfn,
                          vma->vm_end - vma->vm_start,
                          vma->vm_page_prot);
    if (!ret)
        printk(KERN_INFO "dwq: pid %d mmap at %lx\n",
               current->pid, vma->vm_start);
    return ret;
}

static const struct file_operations dwq_fops = {
    .owner          = THIS_MODULE,
    .open           = dwq_open,
    .release        = dwq_release,
    .read           = dwq_read,
    .write          = dwq_write,
    .unlocked_ioctl = dwq_ioctl,
    .mmap           = dwq_mmap,
};

/* ------------------------------------------------------------------ */
/* module init / exit                                                   */
/* ------------------------------------------------------------------ */
static int __init dwq_init(void)
{
    int ret, i;

    ret = alloc_chrdev_region(&g_dwq.devno, 0, 1, "dwq");
    if (ret < 0)
        return ret;

    cdev_init(&g_dwq.cdev, &dwq_fops);
    g_dwq.cdev.owner = THIS_MODULE;
    ret = cdev_add(&g_dwq.cdev, g_dwq.devno, 1);
    if (ret < 0) {
        unregister_chrdev_region(g_dwq.devno, 1);
        return ret;
    }

    mutex_init(&g_dwq.lock);

    /* 初始化 handle 表 */
    for (i = 0; i < NR_HANDLES; i++)
        mutex_init(&g_dwq.handles[i].lock);

    /* handle=0 内置 workqueue 服务 */
    g_dwq.handles[DWQ_HANDLE_DEFAULT].registered = 1;
    strncpy(g_dwq.handles[DWQ_HANDLE_DEFAULT].name, "builtin-wq",
            sizeof(g_dwq.handles[DWQ_HANDLE_DEFAULT].name));

    INIT_LIST_HEAD(&g_dwq.wq_pending);
    spin_lock_init(&g_dwq.wq_lock);

    g_dwq.wq = create_workqueue("dwq_wq");
    if (!g_dwq.wq) {
        cdev_del(&g_dwq.cdev);
        unregister_chrdev_region(g_dwq.devno, 1);
        return -ENOMEM;
    }
    INIT_DELAYED_WORK(&g_dwq.dwork, delay_func);

    /* shared memory */
    g_dwq.shm_page = alloc_page(GFP_KERNEL | __GFP_ZERO);
    if (!g_dwq.shm_page) {
        destroy_workqueue(g_dwq.wq);
        cdev_del(&g_dwq.cdev);
        unregister_chrdev_region(g_dwq.devno, 1);
        return -ENOMEM;
    }
    g_dwq.shm = (struct dwq_shm *)page_address(g_dwq.shm_page);

    /* udev 自动创建 /dev/dwq */
    g_dwq.cls = class_create("dwq");
    if (IS_ERR(g_dwq.cls)) {
        ret = PTR_ERR(g_dwq.cls);
        goto err_cls;
    }
    g_dwq.dev = device_create(g_dwq.cls, NULL, g_dwq.devno, NULL, "dwq");
    if (IS_ERR(g_dwq.dev)) {
        ret = PTR_ERR(g_dwq.dev);
        class_destroy(g_dwq.cls);
        goto err_cls;
    }

    printk(KERN_INFO "dwq: loaded major=%d /dev/dwq ready\n",
           MAJOR(g_dwq.devno));
    printk(KERN_INFO "dwq: handle=0 → builtin workqueue\n");
    printk(KERN_INFO "dwq: handle=1~7 → ioctl(DWQ_IOC_REGISTER) to register\n");
    return 0;

err_cls:
    __free_page(g_dwq.shm_page);
    destroy_workqueue(g_dwq.wq);
    cdev_del(&g_dwq.cdev);
    unregister_chrdev_region(g_dwq.devno, 1);
    return ret;
}

static void __exit dwq_exit(void)
{
    struct dwq_transaction *tx, *tmp;
    unsigned long flags;

    cancel_delayed_work_sync(&g_dwq.dwork);
    flush_workqueue(g_dwq.wq);
    destroy_workqueue(g_dwq.wq);

    spin_lock_irqsave(&g_dwq.wq_lock, flags);
    list_for_each_entry_safe(tx, tmp, &g_dwq.wq_pending, node) {
        list_del(&tx->node);
        kfree(tx);
    }
    spin_unlock_irqrestore(&g_dwq.wq_lock, flags);

    device_destroy(g_dwq.cls, g_dwq.devno);
    class_destroy(g_dwq.cls);
    cdev_del(&g_dwq.cdev);
    unregister_chrdev_region(g_dwq.devno, 1);
    __free_page(g_dwq.shm_page);
    printk(KERN_INFO "dwq: unloaded\n");
}

module_init(dwq_init);
module_exit(dwq_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lewiyon <lewiyon@hotmail.com>");
MODULE_DESCRIPTION("Char device – Binder-like service registration and routing");
