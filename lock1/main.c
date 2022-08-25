#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include "other.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("gzhang");
MODULE_DESCRIPTION("this is a test module");

// static DEFINE_SPINLOCK(threads_lock);
static spinlock_t threads_lock;

static void threads_lock_init(void) {
    spin_lock_init(&threads_lock);
}

struct our_data {
    int count1;
    int count2;
};

static struct our_data my_data;

static void show_my_data(void) {
    printk("count1=%d, count2=%d\n", my_data.count1, my_data.count2);
}

#define MAX_THREAD  10

static struct task_struct *threads[MAX_THREAD];

static int thread_do(void *data) {
    printk("run ... \n");

    while (!kthread_should_stop()) {
        spin_lock(&threads_lock);
        my_data.count1++;
        my_data.count2 += 10;
        spin_unlock(&threads_lock);
        msleep(10);
    }
    return 0;
}

static int create_threads(void) {
    int i;

    for (i = 0; i < MAX_THREAD; i++) {
        struct task_struct*thread; 
        thread = kthread_run(thread_do, NULL, "thread-%d", i);
        if (IS_ERR(thread)) {
            return -1;
        }

        threads[i] = thread;
    }

    return 0;
}

static int cleanup_threads(void) {
    int i;

    for (i = 0; i < MAX_THREAD; i++) {
        if (threads[i]) {
            kthread_stop(threads[i]);
        }
    }

    return 0;
}

static __init int minit(void) {
    printk("call %s.\n", __FUNCTION__);
    threads_lock_init();
    
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
    show_my_data();
}

module_init(minit);
module_exit(mexit);
