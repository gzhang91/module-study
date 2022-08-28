#include <linux/module.h>
#include "other.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("gzhang");
MODULE_DESCRIPTION("this is a test module");

static unsigned int testpar = 0;
module_param(testpar, uint, S_IRUGO | S_IWUSR);

static __init int minit(void) {
    other_function();
    printk("testpar=%d\n", testpar);
    printk("call %s.\n", __FUNCTION__);
    return 0;
}

static __exit void mexit(void) {
    printk("exit function ...\n");
}

module_init(minit);
module_exit(mexit);
