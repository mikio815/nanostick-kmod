#include <linux/tty.h>
#include <linux/tty_ldisc.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include "ns_ldisc.h"

static int ns_ldisc_open(struct tty_struct *tty) {
    pr_info("[nanostick] ldisc open\n");
    /* Allow input buffering; 0 can drop all data. */
    tty->receive_room = 65536;
    return 0;
}

static void ns_ldisc_close(struct tty_struct *tty) {
    pr_info("[nanostick] ldisc close\n");
}

static void ns_ldisc_receive_buf(struct tty_struct *tty, const u8 *cp, const u8 *fp, size_t count) {
    pr_info("[nanostick] recv %zu bytes\n", count);
}

static struct tty_ldisc_ops ns_ldisc_ops = {
    .owner = THIS_MODULE,
    .name = "ns_ldisc",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,0,0)
    .num = NS_LDISC_ID,
#endif
    .open = ns_ldisc_open,
    .close = ns_ldisc_close,
    .receive_buf = ns_ldisc_receive_buf,
};

int ns_ldisc_init(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,0,0)
    return tty_register_ldisc(&ns_ldisc_ops);
#else
    return tty_register_ldisc(NS_LDISC_ID, &ns_ldisc_ops);
#endif
}

void ns_ldisc_exit(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,0,0)
    tty_unregister_ldisc(&ns_ldisc_ops);
#else
    tty_unregister_ldisc(NS_LDISC_ID);
#endif
}
