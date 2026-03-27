/**********************************************
 * Author: lewiyon@hotmail.com
 * File name: kthread_worker.c
 * Description: kthread_worker demo – updated for Linux 6.x
 *
 * Old API (3.x)           → New API (4.9+)
 * -----------------------    ----------------------------
 * __init_kthread_worker()  → kthread_init_worker()
 * init_kthread_work()      → kthread_init_work()
 * queue_kthread_work()     → kthread_queue_work()
 * flush_kthread_work()     → kthread_flush_work()
 * kthread_work_func_t      → void (*)(struct kthread_work *)
 * struct kthread_work.done → removed
 *
 * Date: 2011-12-24 (updated for 6.x)
 *********************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/init.h>

/* kthread_worker + kthread_work are declared in <linux/kthread.h> */
static DEFINE_KTHREAD_WORKER(buffer_toggle_worker);
static struct kthread_work   buffer_toggle_work;
static struct task_struct   *buffer_toggle_thread;

/* work function signature: (struct kthread_work *), NOT work_struct */
static void osd_toggle_buffer(struct kthread_work *work)
{
    int i;
    printk(KERN_INFO "kthread_worker: osd_toggle_buffer start\n");
    for (i = 0; i < 3; i++) {
        printk(KERN_INFO "kthread_worker: step %d/3\n", i + 1);
        msleep(1000);
    }
    printk(KERN_INFO "kthread_worker: osd_toggle_buffer done\n");
}

static int __init example_init(void)
{
    int i;

    /* initialise the work item */
    kthread_init_work(&buffer_toggle_work, osd_toggle_buffer);

    /* create and start the kthread */
    buffer_toggle_thread = kthread_run(kthread_worker_fn,
                                       &buffer_toggle_worker,
                                       "aml_buf_toggle");
    if (IS_ERR(buffer_toggle_thread)) {
        printk(KERN_ERR "kthread_worker: kthread_run failed: %ld\n",
               PTR_ERR(buffer_toggle_thread));
        return PTR_ERR(buffer_toggle_thread);
    }

    printk(KERN_INFO "kthread_worker: thread started\n");

    for (i = 0; i < 3; i++) {
        printk(KERN_INFO "kthread_worker: init loop i=%d\n", i);
        msleep(100);
    }

    /* queue the work to be executed by the kthread */
    kthread_queue_work(&buffer_toggle_worker, &buffer_toggle_work);
    printk(KERN_INFO "kthread_worker: work queued\n");

    return 0;
}

static void __exit example_exit(void)
{
    /* wait for any in-flight work to finish, then stop the thread */
    kthread_flush_worker(&buffer_toggle_worker);
    kthread_stop(buffer_toggle_thread);
    printk(KERN_INFO "kthread_worker: module unloaded\n");
}

module_init(example_init);
module_exit(example_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lewiyon <lewiyon@hotmail.com>");
MODULE_DESCRIPTION("kthread_worker demo updated for Linux 6.x");
