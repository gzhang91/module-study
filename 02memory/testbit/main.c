#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include "other.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("gzhang");
MODULE_DESCRIPTION("this is a test module");

#define MAX_THREAD  10

static unsigned long my_bits;

static void show_my_data(void) {
    int i, bits = sizeof(my_bits) * 8;

    for (i = 0; i < bits; i++) {
        int set = test_bit(i, &my_bits);

        if (set) {
            printk("bit %d is set\n", i);
        }
    }
}

static struct task_struct *threads[MAX_THREAD];

static int thread_do(void *data) {

    long i = (long)data;
    printk("run thread-%ld... \n", i);

    set_bit(i, &my_bits);

    while (!kthread_should_stop()) {
        msleep(10);
    }

    return 0;
}

static int create_threads(void) {
    int i;

    for (i = 0; i < MAX_THREAD; i++) {
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

    for (i = 0; i < MAX_THREAD; i++) {
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
    show_my_data();
}

module_init(minit);
module_exit(mexit);
