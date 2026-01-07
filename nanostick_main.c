#include <linux/module.h>
#include <linux/init.h>

#include "ns_input.h"
#include "ns_ldisc.h"

static int __init ns_init(void)
{
    int err;

    pr_info("[nanostick] init\n");

    err = ns_input_init();
    if (err)
        return err;

    err = ns_ldisc_init();
    if (err) {
        ns_input_exit();
        return err;
    }

    pr_info("[nanostick] loaded successfully\n");
    return 0;
}

static void __exit ns_exit(void)
{
    pr_info("[nanostick] exit\n");

    ns_ldisc_exit();
    ns_input_exit();
}

module_init(ns_init);
module_exit(ns_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("mikio815");
MODULE_DESCRIPTION("NanoStick kernel module skeleton");
