#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("gzhang");
MODULE_DESCRIPTION("this is a test module");

#define MAX_LIST  10

struct our_data {
    int i;
    struct hlist_node list;
};

static void list_test(void) {
    int i;
    struct our_data *node;
    struct hlist_node *next;
    HLIST_HEAD(lhead);

    for (i = 0; i < MAX_LIST; i++) {
        struct our_data *data;

        data = kmalloc(sizeof(*data), GFP_KERNEL);
        if (!data) {
            goto clean;
        }

        data->i = i;
        hlist_add_head(&data->list, &lhead);
    }

    hlist_for_each_entry(node, &lhead, list)
        printk("list entry: %d.\n", node->i);

clean:
    hlist_for_each_entry_safe(node, next, &lhead, list)
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
