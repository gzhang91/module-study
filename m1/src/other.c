#include <linux/module.h>

void other_function(void) {
    printk("call %s.\n", __FUNCTION__);
}