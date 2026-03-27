/**********************************************
 * Author: lewiyon@hotmail.com
 * File name: delay_work_queue.c
 * Description: char device driver – Binder-like IPC with service
 *   registration, per-fd transaction routing, callback and listener.
 *
 *   Architecture
 *   ────────────
 *   handle 表  (类比 binder_node)
 *     handle=0  : 内置 workqueue
 *     handle=1~7: ioctl(REGISTER) 注册用户空间 server
 *     每个 handle 还维护一个 subscribers 列表（listener 模式）
 *
 *   per-fd context  dwq_fd_ctx  (类比 binder_thread)
 *     done_list    : 已完成的回复/事件，client read() 在此等待
 *     pending_list : 待处理请求，server read() 在此等待
 *     subscribed   : 本 fd 订阅的 handle 列表，close 时自动清理
 *
 *   三种通信模式
 *   ──────────
 *   1. Request-Reply (同步)
 *      client write(handle=N) → server read() → server write(REPLY)
 *      → client read()
 *
 *   2. Callback (异步)
 *      client ioctl(REGISTER, handle=M) 注册回调端点
 *      client write(handle=N, callback_handle=M)
 *      server read() 拿到 callback_handle=M
 *      server write(handle=M, cmd=EVENT) 反向调用
 *      client 另一线程 read() 拿到事件
 *      类比 Binder：client 把自己的 IBinder 传给 server
 *
 *   3. Listener / Broadcast (发布-订阅)
 *      client ioctl(SUBSCRIBE, handle=N) 订阅
 *      client read() 阻塞等事件
 *      server ioctl(BROADCAST, handle=N, data) 广播
 *      驱动遍历 subscribers，逐一放事件 + wake_up
 *      类比 Binder：IServiceCallback / DeathRecipient
 *
 * Date: 2011-12-24 (callback + listener support)
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
    struct dwq_msg     msg;
    struct dwq_reply   reply;
    struct dwq_fd_ctx *caller;       /* 发起方，回复时精确唤醒          */
};

/* ------------------------------------------------------------------ */
/* subscriber node  (类比 DeathRecipient 链表节点)                     */
/* ------------------------------------------------------------------ */
struct dwq_subscriber {
    struct list_head   node;         /* 挂在 dwq_handle_entry.subscribers */
    struct list_head   back_node;    /* 挂在 dwq_fd_ctx.subscribed，反向引用 */
    struct dwq_fd_ctx *ctx;
    __u32              handle;       /* 订阅的是哪个 handle               */
};

/* ------------------------------------------------------------------ */
/* per-fd context  (类比 binder_thread + binder_proc)                  */
/* ------------------------------------------------------------------ */
struct dwq_fd_ctx {
    wait_queue_head_t  wait;
    atomic_t           tx_id_counter;

    /* client 侧：已完成回复/事件队列 */
    struct list_head   done_list;
    spinlock_t         done_lock;

    /* server 侧：待处理请求队列（未被 read 取走） */
    struct list_head   pending_list;
    spinlock_t         pending_lock;

    /* server 侧：已被 read 取走、等待 write(REPLY) 的事务 */
    struct list_head   dispatched_list;
    spinlock_t         dispatched_lock;

    /* 角色 */
    int                is_server;
    __u32              server_handle;

    /* listener：本 fd 订阅的 handle 集合（close 时自动 unsub）*/
    struct list_head   subscribed;
    spinlock_t         subscribed_lock;
};

/* ------------------------------------------------------------------ */
/* handle 表项  (类比 binder_node)                                     */
/* ------------------------------------------------------------------ */
struct dwq_handle_entry {
    int                registered;
    char               name[32];
    struct dwq_fd_ctx *server_ctx;
    struct mutex       lock;
    /* listener 订阅者列表 */
    struct list_head   subscribers;  /* 元素: dwq_subscriber.node       */
};

#define NR_HANDLES  DWQ_HANDLE_MAX

/* ------------------------------------------------------------------ */
/* shared memory (mmap)                                                 */
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

    struct dwq_handle_entry  handles[NR_HANDLES];

    /* handle=0 内置 workqueue */
    struct workqueue_struct *wq;
    struct delayed_work      dwork;
    spinlock_t               wq_lock;
    struct list_head         wq_pending;

    struct page             *shm_page;
    struct dwq_shm          *shm;

    struct mutex             lock;
};

static struct dwq_dev g_dwq;

/* ------------------------------------------------------------------ */
/* helper: 把事件放入 fd_ctx 的 done_list 并唤醒                       */
/* ------------------------------------------------------------------ */
static void deliver_event(struct dwq_fd_ctx *ctx,
                           __u32 handle, __u32 event_id,
                           __s32 result,
                           const char *data, __u32 data_len,
                           __u32 tx_id)
{
    struct dwq_transaction *ev;
    unsigned long flags;

    ev = kzalloc(sizeof(*ev), GFP_ATOMIC);
    if (!ev)
        return;

    ev->reply.handle   = handle;
    ev->reply.cmd      = DWQ_CMD_EVENT;
    ev->reply.tx_id    = tx_id;
    ev->reply.result   = result;
    ev->reply.data_len = min(data_len, (__u32)DWQ_MAX_DATA);
    if (data && data_len)
        memcpy(ev->reply.data, data, ev->reply.data_len);
    /* reuse data[0..3] as event_id */
    if (ev->reply.data_len == 0) {
        memcpy(ev->reply.data, &event_id, sizeof(event_id));
        ev->reply.data_len = sizeof(event_id);
    }

    spin_lock_irqsave(&ctx->done_lock, flags);
    list_add_tail(&ev->node, &ctx->done_list);
    spin_unlock_irqrestore(&ctx->done_lock, flags);
    wake_up_interruptible(&ctx->wait);
}

/* ------------------------------------------------------------------ */
/* 内置 workqueue handler (handle=0)                                   */
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

    printk(KERN_INFO "dwq: [wq] tx_id=%u data=[%.*s]\n",
           tx->msg.tx_id, (int)tx->msg.data_len, tx->msg.data);

    for (i = 0; i < 3; i++) {
        printk(KERN_INFO "dwq: [wq] step %d/3\n", i + 1);
        msleep(1000);
    }

    memset(&tx->reply, 0, sizeof(tx->reply));
    tx->reply.handle   = tx->msg.handle;
    tx->reply.cmd      = tx->msg.cmd;
    tx->reply.tx_id    = tx->msg.tx_id;
    tx->reply.result   = DWQ_RESULT_OK;
    snprintf(tx->reply.data, DWQ_MAX_DATA, "wq done tx_id=%u", tx->msg.tx_id);
    tx->reply.data_len = strlen(tx->reply.data);

    spin_lock_irqsave(&tx->caller->done_lock, flags);
    list_add_tail(&tx->node, &tx->caller->done_list);
    spin_unlock_irqrestore(&tx->caller->done_lock, flags);
    wake_up_interruptible(&tx->caller->wait);

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
    INIT_LIST_HEAD(&ctx->dispatched_list);
    spin_lock_init(&ctx->dispatched_lock);
    INIT_LIST_HEAD(&ctx->subscribed);
    spin_lock_init(&ctx->subscribed_lock);

    filp->private_data = ctx;
    printk(KERN_INFO "dwq: open pid=%d\n", current->pid);
    return 0;
}

static int dwq_release(struct inode *inode, struct file *filp)
{
    struct dwq_fd_ctx      *ctx = filp->private_data;
    struct dwq_transaction *tx, *tmp_tx;
    struct dwq_subscriber  *sub, *tmp_sub;
    unsigned long           flags;

    /* 1. server 自动注销 handle */
    if (ctx->is_server) {
        __u32 h = ctx->server_handle;
        if (h < NR_HANDLES) {
            mutex_lock(&g_dwq.handles[h].lock);
            g_dwq.handles[h].server_ctx = NULL;
            g_dwq.handles[h].registered = 0;
            mutex_unlock(&g_dwq.handles[h].lock);
            printk(KERN_INFO "dwq: handle %u auto-unregistered\n", h);
        }
    }

    /* 2. 自动取消所有订阅 */
    spin_lock_irqsave(&ctx->subscribed_lock, flags);
    list_for_each_entry_safe(sub, tmp_sub, &ctx->subscribed, back_node) {
        list_del(&sub->back_node);
        spin_unlock_irqrestore(&ctx->subscribed_lock, flags);

        /* 从 handle 的 subscribers 列表里移除 */
        mutex_lock(&g_dwq.handles[sub->handle].lock);
        list_del(&sub->node);
        mutex_unlock(&g_dwq.handles[sub->handle].lock);

        kfree(sub);
        spin_lock_irqsave(&ctx->subscribed_lock, flags);
    }
    spin_unlock_irqrestore(&ctx->subscribed_lock, flags);

    /* 3. 释放 done_list */
    spin_lock_irqsave(&ctx->done_lock, flags);
    list_for_each_entry_safe(tx, tmp_tx, &ctx->done_list, node) {
        list_del(&tx->node);
        kfree(tx);
    }
    spin_unlock_irqrestore(&ctx->done_lock, flags);

    /* 4. 释放 pending_list，通知 caller 服务关闭 */
    spin_lock_irqsave(&ctx->pending_lock, flags);
    list_for_each_entry_safe(tx, tmp_tx, &ctx->pending_list, node) {
        list_del(&tx->node);
        tx->reply.handle   = tx->msg.handle;
        tx->reply.cmd      = tx->msg.cmd;
        tx->reply.tx_id    = tx->msg.tx_id;
        tx->reply.result   = DWQ_RESULT_ENODEV;
        snprintf(tx->reply.data, DWQ_MAX_DATA, "server closed");
        tx->reply.data_len = strlen(tx->reply.data);

        spin_lock(&tx->caller->done_lock);
        list_add_tail(&tx->node, &tx->caller->done_list);
        spin_unlock(&tx->caller->done_lock);
        wake_up_interruptible(&tx->caller->wait);
    }
    spin_unlock_irqrestore(&ctx->pending_lock, flags);

    /* 5. 释放 dispatched_list（已 read 但未 reply 的事务），同样通知 caller */
    spin_lock_irqsave(&ctx->dispatched_lock, flags);
    list_for_each_entry_safe(tx, tmp_tx, &ctx->dispatched_list, node) {
        list_del(&tx->node);
        tx->reply.handle   = tx->msg.handle;
        tx->reply.cmd      = tx->msg.cmd;
        tx->reply.tx_id    = tx->msg.tx_id;
        tx->reply.result   = DWQ_RESULT_ENODEV;
        snprintf(tx->reply.data, DWQ_MAX_DATA, "server closed");
        tx->reply.data_len = strlen(tx->reply.data);

        spin_lock(&tx->caller->done_lock);
        list_add_tail(&tx->node, &tx->caller->done_list);
        spin_unlock(&tx->caller->done_lock);
        wake_up_interruptible(&tx->caller->wait);
    }
    spin_unlock_irqrestore(&ctx->dispatched_lock, flags);

    kfree(ctx);
    printk(KERN_INFO "dwq: close pid=%d\n", current->pid);
    return 0;
}

/*
 * write() 三种路径：
 *   DWQ_CMD_REPLY  : server 回复 client request
 *   DWQ_CMD_EVENT  : server 反向回调 client (callback 模式)
 *   其他 cmd       : client 发起请求
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

    /* ---- path 1: server 回复 (BC_REPLY) ---- */
    if (msg.cmd == DWQ_CMD_REPLY) {
        if (!ctx->is_server)
            return -EPERM;

        tx = NULL;
        /* 从 dispatched_list 查（tx 已被 read() 摘出，等待 reply） */
        spin_lock_irqsave(&ctx->dispatched_lock, flags);
        {
            struct dwq_transaction *t;
            list_for_each_entry(t, &ctx->dispatched_list, node) {
                if (t->msg.tx_id == msg.tx_id) {
                    tx = t;
                    list_del(&tx->node);
                    break;
                }
            }
        }
        spin_unlock_irqrestore(&ctx->dispatched_lock, flags);

        if (!tx) {
            printk(KERN_ERR "dwq: REPLY unknown tx_id=%u\n", msg.tx_id);
            return -EINVAL;
        }

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

        printk(KERN_INFO "dwq: REPLY tx_id=%u → client\n", msg.tx_id);
        return (ssize_t)count;
    }

    /* ---- path 2: server 反向 callback (DWQ_CMD_EVENT) ---- */
    /* server 向 client 注册的 callback_handle 发送事件        */
    if (msg.cmd == DWQ_CMD_EVENT) {
        struct dwq_fd_ctx *target_ctx;

        if (msg.handle >= NR_HANDLES)
            return -EINVAL;

        mutex_lock(&dev->handles[msg.handle].lock);
        target_ctx = dev->handles[msg.handle].server_ctx;
        mutex_unlock(&dev->handles[msg.handle].lock);

        if (!target_ctx) {
            printk(KERN_ERR "dwq: EVENT handle %u no listener\n", msg.handle);
            return -ENODEV;
        }

        deliver_event(target_ctx, msg.handle, 0,
                      DWQ_RESULT_OK, msg.data, msg.data_len, msg.tx_id);

        /* callback 模式：tx 不走 done_list，直接从 dispatched_list 摘出并 free */
        if (ctx->is_server) {
            struct dwq_transaction *t, *tmp_t;
            spin_lock_irqsave(&ctx->dispatched_lock, flags);
            list_for_each_entry_safe(t, tmp_t, &ctx->dispatched_list, node) {
                if (t->msg.tx_id == msg.tx_id) {
                    list_del(&t->node);
                    kfree(t);
                    break;
                }
            }
            spin_unlock_irqrestore(&ctx->dispatched_lock, flags);
        }
        printk(KERN_INFO "dwq: EVENT handle=%u → callback client\n",
               msg.handle);
        return (ssize_t)count;
    }

    /* ---- path 3: client 发起请求 (BC_TRANSACTION) ---- */
    if (msg.handle >= NR_HANDLES)
        return -EINVAL;

    tx = kzalloc(sizeof(*tx), GFP_KERNEL);
    if (!tx)
        return -ENOMEM;

    msg.tx_id = (__u32)atomic_inc_return(&ctx->tx_id_counter);
    memcpy(&tx->msg, &msg, sizeof(msg));
    tx->caller = ctx;

    mutex_lock(&dev->handles[msg.handle].lock);

    if (dev->handles[msg.handle].registered &&
        dev->handles[msg.handle].server_ctx != NULL) {
        struct dwq_fd_ctx *srv = dev->handles[msg.handle].server_ctx;
        mutex_unlock(&dev->handles[msg.handle].lock);

        spin_lock_irqsave(&srv->pending_lock, flags);
        list_add_tail(&tx->node, &srv->pending_list);
        spin_unlock_irqrestore(&srv->pending_lock, flags);
        wake_up_interruptible(&srv->wait);

        printk(KERN_INFO "dwq: TX handle=%u cmd=%u tx_id=%u cb=%u → server\n",
               msg.handle, msg.cmd, msg.tx_id, msg.callback_handle);

    } else if (msg.handle == DWQ_HANDLE_DEFAULT) {
        mutex_unlock(&dev->handles[msg.handle].lock);

        spin_lock_irqsave(&dev->wq_lock, flags);
        list_add_tail(&tx->node, &dev->wq_pending);
        queue_delayed_work(dev->wq, &dev->dwork, 0);
        spin_unlock_irqrestore(&dev->wq_lock, flags);

        printk(KERN_INFO "dwq: TX handle=0 tx_id=%u → workqueue\n", msg.tx_id);

    } else {
        mutex_unlock(&dev->handles[msg.handle].lock);
        kfree(tx);
        return -ENODEV;
    }

    return (ssize_t)count;
}

/*
 * read() 双重职责：
 *   client (is_server=0): 等 done_list → dwq_reply
 *   server (is_server=1): 等 pending_list → dwq_msg
 */
static ssize_t dwq_read(struct file *filp, char __user *buf,
                         size_t count, loff_t *ppos)
{
    struct dwq_fd_ctx      *ctx = filp->private_data;
    struct dwq_transaction *tx  = NULL;
    unsigned long           flags;
    int ret;

    if (ctx->is_server) {
        if (count < sizeof(struct dwq_msg))
            return -EINVAL;

        ret = wait_event_interruptible(ctx->wait,
                                       !list_empty(&ctx->pending_list));
        if (ret)
            return ret;

        spin_lock_irqsave(&ctx->pending_lock, flags);
        if (!list_empty(&ctx->pending_list)) {
            tx = list_first_entry(&ctx->pending_list,
                                  struct dwq_transaction, node);
            list_del(&tx->node);   /* 从 pending 摘出 */
        }
        spin_unlock_irqrestore(&ctx->pending_lock, flags);

        if (!tx)
            return -EAGAIN;

        /* 放入 dispatched_list，等 write(REPLY) 按 tx_id 匹配后 free */
        spin_lock_irqsave(&ctx->dispatched_lock, flags);
        list_add_tail(&tx->node, &ctx->dispatched_list);
        spin_unlock_irqrestore(&ctx->dispatched_lock, flags);

        if (copy_to_user(buf, &tx->msg, sizeof(tx->msg)))
            return -EFAULT;

        printk(KERN_INFO "dwq: server got tx_id=%u cmd=%u cb=%u\n",
               tx->msg.tx_id, tx->msg.cmd, tx->msg.callback_handle);
        return (ssize_t)sizeof(struct dwq_msg);

    } else {
        /* client 等回复或事件（done_list 里混放，cmd 字段区分） */
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

        printk(KERN_INFO "dwq: client got cmd=%u tx_id=%u result=%d\n",
               tx->reply.cmd, tx->reply.tx_id, tx->reply.result);
        kfree(tx);
        return (ssize_t)sizeof(struct dwq_reply);
    }
}

static long dwq_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct dwq_fd_ctx *ctx = filp->private_data;
    struct dwq_dev    *dev = &g_dwq;

    switch (cmd) {

    /* ---- 服务注册 ---- */
    case DWQ_IOC_REGISTER: {
        struct dwq_reg_info info;
        __u32 h;

        if (copy_from_user(&info, (void __user *)arg, sizeof(info)))
            return -EFAULT;
        h = info.handle;
        if (h == DWQ_HANDLE_DEFAULT || h >= NR_HANDLES)
            return -EINVAL;

        mutex_lock(&dev->handles[h].lock);
        if (dev->handles[h].server_ctx != NULL) {
            mutex_unlock(&dev->handles[h].lock);
            return -EBUSY;
        }
        dev->handles[h].registered = 1;
        dev->handles[h].server_ctx = ctx;
        strncpy(dev->handles[h].name, info.name,
                sizeof(dev->handles[h].name) - 1);
        mutex_unlock(&dev->handles[h].lock);

        ctx->is_server     = 1;
        ctx->server_handle = h;
        printk(KERN_INFO "dwq: handle %u registered \"%s\" pid=%d\n",
               h, info.name, current->pid);
        break;
    }

    case DWQ_IOC_UNREGISTER: {
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

    /* ---- Listener：订阅广播事件 ---- */
    case DWQ_IOC_SUBSCRIBE: {
        /* arg = handle to subscribe to
         * 类比 registerCallback(IMyCallback cb) */
        __u32 h = (__u32)arg;
        struct dwq_subscriber *sub;

        if (h >= NR_HANDLES)
            return -EINVAL;

        sub = kzalloc(sizeof(*sub), GFP_KERNEL);
        if (!sub)
            return -ENOMEM;

        sub->ctx    = ctx;
        sub->handle = h;

        /* 挂入 handle 的 subscribers 列表 */
        mutex_lock(&dev->handles[h].lock);
        list_add_tail(&sub->node, &dev->handles[h].subscribers);
        mutex_unlock(&dev->handles[h].lock);

        /* 反向记录，close 时自动清理 */
        spin_lock_irq(&ctx->subscribed_lock);
        list_add_tail(&sub->back_node, &ctx->subscribed);
        spin_unlock_irq(&ctx->subscribed_lock);

        printk(KERN_INFO "dwq: pid=%d subscribed handle %u\n",
               current->pid, h);
        break;
    }

    case DWQ_IOC_UNSUBSCRIBE: {
        __u32 h = (__u32)arg;
        struct dwq_subscriber *sub, *tmp;

        spin_lock_irq(&ctx->subscribed_lock);
        list_for_each_entry_safe(sub, tmp, &ctx->subscribed, back_node) {
            if (sub->handle == h) {
                list_del(&sub->back_node);
                spin_unlock_irq(&ctx->subscribed_lock);

                mutex_lock(&dev->handles[h].lock);
                list_del(&sub->node);
                mutex_unlock(&dev->handles[h].lock);

                kfree(sub);
                printk(KERN_INFO "dwq: pid=%d unsubscribed handle %u\n",
                       current->pid, h);
                return 0;
            }
        }
        spin_unlock_irq(&ctx->subscribed_lock);
        return -ENOENT;
    }

    /* ---- Listener：广播事件给所有订阅者 ---- */
    case DWQ_IOC_BROADCAST: {
        /* 类比 server 调 listener.onEvent() 通知所有订阅者 */
        struct dwq_broadcast bc;
        struct dwq_subscriber *sub;

        if (copy_from_user(&bc, (void __user *)arg, sizeof(bc)))
            return -EFAULT;
        if (bc.handle >= NR_HANDLES)
            return -EINVAL;

        mutex_lock(&dev->handles[bc.handle].lock);
        list_for_each_entry(sub, &dev->handles[bc.handle].subscribers, node) {
            deliver_event(sub->ctx, bc.handle, bc.event_id,
                          DWQ_RESULT_OK, bc.data, bc.data_len, 0);
        }
        mutex_unlock(&dev->handles[bc.handle].lock);

        printk(KERN_INFO "dwq: BROADCAST handle=%u event=%u\n",
               bc.handle, bc.event_id);
        break;
    }

    case DWQ_IOC_STATUS: {
        int depth = 0;
        if (ctx->is_server) {
            struct dwq_transaction *t;
            unsigned long f;
            spin_lock_irqsave(&ctx->pending_lock, f);
            list_for_each_entry(t, &ctx->pending_list, node) depth++;
            spin_unlock_irqrestore(&ctx->pending_lock, f);
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
    vm_flags_set(vma, VM_SHARED);
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

    for (i = 0; i < NR_HANDLES; i++) {
        mutex_init(&g_dwq.handles[i].lock);
        INIT_LIST_HEAD(&g_dwq.handles[i].subscribers);
    }

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

    g_dwq.shm_page = alloc_page(GFP_KERNEL | __GFP_ZERO);
    if (!g_dwq.shm_page) {
        destroy_workqueue(g_dwq.wq);
        cdev_del(&g_dwq.cdev);
        unregister_chrdev_region(g_dwq.devno, 1);
        return -ENOMEM;
    }
    g_dwq.shm = (struct dwq_shm *)page_address(g_dwq.shm_page);

    g_dwq.cls = class_create("dwq");
    if (IS_ERR(g_dwq.cls)) {
        ret = PTR_ERR(g_dwq.cls);
        goto err;
    }
    g_dwq.dev = device_create(g_dwq.cls, NULL, g_dwq.devno, NULL, "dwq");
    if (IS_ERR(g_dwq.dev)) {
        ret = PTR_ERR(g_dwq.dev);
        class_destroy(g_dwq.cls);
        goto err;
    }

    printk(KERN_INFO "dwq: loaded major=%d /dev/dwq ready\n",
           MAJOR(g_dwq.devno));
    printk(KERN_INFO "dwq: handle=0 builtin-wq, handle=1~7 user-server\n");
    return 0;

err:
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
MODULE_DESCRIPTION("Char device – Binder-like IPC with callback and listener");
