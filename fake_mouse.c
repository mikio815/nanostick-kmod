#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include "ns_action.h"

static struct input_dev *fm_dev;
static struct task_struct *fm_thread;

static int fm_thread_fn(void *data)
{
    while (!kthread_should_stop()) {
        struct ns_action a = {
            .type = NS_ACT_MOVE,
            .x = 1,
            .y = 0,
        };

        ns_emit_action(fm_dev, &a);

        msleep(10);
    }
    return 0;
}

static int __init fm_init(void)
{
    int err;

    pr_info("[fake_mouse] init\n");

    fm_dev = input_allocate_device();
    if (!fm_dev)
        return -ENOMEM;

    fm_dev->name = "FakeKernelMouse";
    fm_dev->id.bustype = BUS_VIRTUAL;

    // イベント種類
    set_bit(EV_REL,  fm_dev->evbit);
    set_bit(EV_KEY,  fm_dev->evbit);
    set_bit(EV_SYN,  fm_dev->evbit);

    // 相対移動
    set_bit(REL_X, fm_dev->relbit);
    set_bit(REL_Y, fm_dev->relbit);
    set_bit(REL_WHEEL, fm_dev->relbit);

    // ボタン
    set_bit(BTN_LEFT,  fm_dev->keybit);
    set_bit(BTN_RIGHT, fm_dev->keybit);
    set_bit(KEY_LEFTCTRL, fm_dev->keybit);

    err = input_register_device(fm_dev);
    if (err) {
        input_free_device(fm_dev);
        pr_err("[fake_mouse] input_register_device failed\n");
        return err;
    }

    fm_thread = kthread_run(fm_thread_fn, NULL, "fake_mouse_thread");
    if (IS_ERR(fm_thread)) {
        err = PTR_ERR(fm_thread);
        input_unregister_device(fm_dev);
        pr_err("[fake_mouse] kthread_run failed\n");
        return err;
    }

    pr_info("[fake_mouse] loaded successfully\n");
    return 0;
}

static void __exit fm_exit(void)
{
    pr_info("[fake_mouse] exit\n");

    if (fm_thread && !IS_ERR(fm_thread))
        kthread_stop(fm_thread);

    if (fm_dev)
        input_unregister_device(fm_dev);
}

module_init(fm_init);
module_exit(fm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("mikio815");
MODULE_DESCRIPTION("Virtual mouse kernel module skeleton");
