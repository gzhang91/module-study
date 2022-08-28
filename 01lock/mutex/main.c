#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>
#include "other.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("gzhang");
MODULE_DESCRIPTION("this is a test module");

static unsigned long reader_bitmap;

static void set_reader_number(int reader) {
    reader_bitmap = 0;
    while (reader) {
        reader_bitmap |= (1 << --reader);
    }
}

struct our_data {
    int count1;
    int count2;
};

static struct our_data my_data;
static struct our_data __rcu *pmy_data = &my_data;

static void show_my_data(struct our_data *data) {
    printk("count1=%d, count2=%d\n", data->count1, data->count2);
}

// static DEFINE_MUTEX(my_lock);
static DEFINE_SEMAPHORE(my_sem);

static void reader_do(void) {
    struct our_data *data;

    rcu_read_lock();
    data = rcu_dereference(pmy_data);
    show_my_data(data);
    rcu_read_unlock();
}

static void writer_do(void) {
#if 0
    struct our_data *data, *tmp = pmy_data;
    data = kmalloc(sizeof *data, GFP_KERNEL);
    if (!data) {
        return;
    }

    memcpy(data, pmy_data, sizeof(*data));
    data->count1++;
    data->count2 += 10;

    rcu_assign_pointer(pmy_data, data);

    if (tmp != &my_data) {
        synchronize_rcu();
        kfree(tmp);
    }
#endif
    // mutex_lock(&my_lock);
    down(&my_sem);
    my_data.count1 ++;
    my_data.count2 += 10;
    up(&my_sem);
    // mutex_unlock(&my_lock);
}

#define MAX_THREAD  10

static struct task_struct *threads[MAX_THREAD];

static int thread_do(void *data) {

    long i = (long)data;
    int reader = (reader_bitmap & (1 << i));
    printk("run(%ld) %s ... \n", i, reader ? "reader" : "writer");

    while (!kthread_should_stop()) {
        if (reader) {
            reader_do();
        } else {
            writer_do();
        }

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
    set_reader_number(0);
    
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
    show_my_data(pmy_data);
}

module_init(minit);
module_exit(mexit);
