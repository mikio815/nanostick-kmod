#include <tty.h>
#include <tty_ldisc.h>
#include <kernel.h>
#include "ns_ldisc.h"

static int ns_ldisc_open(struct tty_struct *tty) {
    pr_info("[nanostick] ldisc open\n");
    return 0;
}

static void ns_ldisc_open(struct tty_struct *tty) {
    pr_info("[nanostick] ldisc close\n");
}

static void ns_ldisc_receive_buf(struct tty_struct *tty, const u8 *cp, const u8 *fp, size_t count) {
    pr_debug("[nanostick] recv %d bytes\n", count);
}

static struct tty_ldisc_ops ns_ldisc_ops = {
    .owner = THIS_MODULE,
    .name = "ns_ldisc",
    .open = ns_ldisc_open,
    .close = ns_ldisc_close,
    .receive_buf = ns_ldisc_receive_buf,
};

int ns_ldisc_init(void)
{
    return tty_register_ldisc(NS_LDISC_ID, &ns_ldisc_ops);
}

void ns_ldisc_exit(void)
{
    tty_unregister_ldisc(NS_LDISC_ID);
}

