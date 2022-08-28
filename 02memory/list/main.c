#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include "other.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("gzhang");
MODULE_DESCRIPTION("this is a test module");

#define MAX_LIST  10

struct our_data {
    int i;
    struct list_head list;
};

static void list_test(void) {
    int i;
    struct our_data *node, *next;
    LIST_HEAD(list);

    for (i = 0; i < MAX_LIST; i++) {
        struct our_data *data;

        data = kmalloc(sizeof(*data), GFP_KERNEL);
        if (!data) {
            goto clean;
        }

        data->i = i;
        list_add(&data->list, &list);
    }

    list_for_each_entry(node, &list, list)
        printk("list entry: %d.\n", node->i);

clean:
    list_for_each_entry_safe(node, next, &list, list)
        kfree(node);
}

static __init int minit(void) {
    printk("call %s.\n", __FUNCTION__);

    list_test();

    return 0; 
}

static __exit void mexit(void) {
    printk("call %s.\n", __FUNCTION__);
}

module_init(minit);
module_exit(mexit);
