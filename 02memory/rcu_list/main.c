#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/rculist.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("gzhang");
MODULE_DESCRIPTION("this is a test module");

#define MAX_KTHREAD  10

struct our_data {
    int i;
    struct list_head list;
    struct rcu_head rcu;
};

static void free_mydata(struct rcu_head *rcu) {
    kfree(container_of(rcu, struct our_data, rcu));
}

static struct task_struct *threads[MAX_KTHREAD];
static LIST_HEAD(rlist);
static DEFINE_MUTEX(mylock);

static void cleanup_my_data(void) {
    struct our_data *node, *next;

    list_for_each_entry_safe(node, next, &rlist, list)
        kfree(node);
}

static void write_list(long i) {
    struct our_data *old = NULL, *data = kmalloc(sizeof(*data), GFP_KERNEL);

    if (!data) {
        return;
    }

    mutex_lock(&mylock);
    if (!list_empty(&rlist)) {
        old = list_last_entry(&rlist, struct our_data, list);
        list_del_rcu(&old->list);
        call_rcu(&old->rcu, free_mydata);
    }

    data->i = i;
    list_add_rcu(&data->list, &rlist);
    mutex_lock(&mylock);
}

static void read_list(void) {
    struct our_data *data;
    rcu_read_lock();

    list_for_each_entry_rcu(data, &rlist, list)
        printk("entry %d.\n", data->i);
    rcu_read_unlock();

    msleep(1000);
}

static int thread_do(void *data) {

    long i = (long)data;
    printk("run thread-%ld... \n", i);

    while (!kthread_should_stop()) {
        if (i == 0) {
            read_list();
        } else {
            write_list(i);
        }
        
        msleep(10);
    }

    return 0;
}

static int create_threads(void) {
    int i;

    for (i = 0; i < MAX_KTHREAD; i++) {
        struct task_struct*thread; 
        thread = kthread_run(thread_do, (void *)(long)i, "thread-%d", i);
        if (IS_ERR(thread)) {
            return -1;
        }

        threads[i] = thread;
    }

    return 0;
}

static int cleanup_threads(void) {
    int i;

    for (i = 0; i < MAX_KTHREAD; i++) {
        if (threads[i]) {
            kthread_stop(threads[i]);
        }
    }

    return 0;
}

static __init int minit(void) {
    printk("call %s.\n", __FUNCTION__);
    
    if (create_threads()) {
        goto err;
    }

    return 0;
err:
    cleanup_threads();
    return -1;
}

static __exit void mexit(void) {
    printk("call %s.\n", __FUNCTION__);
    cleanup_threads();
    cleanup_my_data();
}

module_init(minit);
module_exit(mexit);
