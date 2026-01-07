#include <linux/tty.h>
#include <linux/tty_ldisc.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/slab.h>
#include "ns_ldisc.h"
#include "ns_proto.h"
#include "ns_action.h"
#include "ns_input.h"

struct ns_ldisc_ctx {
    struct ns_proto_state proto;
    u8 prev_buttons;
};

static void ns_ldisc_on_frame(const struct ns_proto_frame *f, void *ctx)
{
    struct tty_struct *tty = ctx;
    struct ns_ldisc_ctx *ld = tty ? tty->disc_data : NULL;
    struct input_dev *dev = ns_input_get_dev();
    int dx, dy_move, scroll = 0, zoom = 0;
    u8 changed;

    pr_info("[nanostick] seq=%u lx=%d ly=%d rx=%d ry=%d btn=%u flg=%u\n",
            f->seq, f->lx, f->ly, f->rx, f->ry, f->buttons, f->flags);

    if (!ld || !dev)
        return;

    /*
     - Y軸 同方向: 上下スクロール
     - X軸 逆方向: ズームイン/アウト
    */
    dx = f->lx + f->rx;
    dy_move = f->ly + f->ry;

    if (f->ly && f->ry &&
        ((f->ly > 0 && f->ry > 0) || (f->ly < 0 && f->ry < 0))) {
        scroll = (f->ly + f->ry) / 2;
        dy_move = 0;
    }

    if (f->lx && f->rx &&
        ((f->lx > 0 && f->rx < 0) || (f->lx < 0 && f->rx > 0))) {
        zoom = (f->lx - f->rx) / 2;
        dx = 0;
    }

    if (dx || dy_move) {
        struct ns_action a = {
            .type = NS_ACT_MOVE,
            .x = dx,
            .y = dy_move,
        };
        ns_emit_action(dev, &a);
    }

    if (scroll) {
        struct ns_action a = {
            .type = NS_ACT_SCROLL,
            .wheel = scroll,
        };
        ns_emit_action(dev, &a);
    }

    if (zoom) {
        struct ns_action a = {
            .type = NS_ACT_ZOOM,
            .wheel = zoom,
        };
        ns_emit_action(dev, &a);
    }

    changed = ld->prev_buttons ^ f->buttons;
    if (changed & 0x01) {
        struct ns_action a = {
            .type = NS_ACT_CLICK,
            .button = BTN_LEFT,
            .pressed = !!(f->buttons & 0x01),
        };
        ns_emit_action(dev, &a);
    }
    if (changed & 0x02) {
        struct ns_action a = {
            .type = NS_ACT_CLICK,
            .button = BTN_RIGHT,
            .pressed = !!(f->buttons & 0x02),
        };
        ns_emit_action(dev, &a);
    }

    ld->prev_buttons = f->buttons;
}

static int ns_ldisc_open(struct tty_struct *tty) {
    struct ns_ldisc_ctx *ld;

    pr_info("[nanostick] ldisc open\n");
    tty->receive_room = 65536;
    ld = kzalloc(sizeof(*ld), GFP_KERNEL);
    if (!ld)
        return -ENOMEM;
    ns_proto_init(&ld->proto);
    ld->prev_buttons = 0;
    tty->disc_data = ld;
    return 0;
}

static void ns_ldisc_close(struct tty_struct *tty) {
    struct ns_ldisc_ctx *ld = tty->disc_data;

    pr_info("[nanostick] ldisc close\n");
    kfree(ld);
    tty->disc_data = NULL;
}

static void ns_ldisc_receive_buf(struct tty_struct *tty, const u8 *cp, const u8 *fp, size_t count) {
    struct ns_ldisc_ctx *ld = tty->disc_data;

    if (!ld)
        return;
    ns_proto_feed(&ld->proto, cp, count, ns_ldisc_on_frame, tty);
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
