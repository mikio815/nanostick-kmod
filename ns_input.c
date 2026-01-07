#include <linux/kernel.h>
#include <linux/input.h>

#include "ns_input.h"

static struct input_dev *ns_dev;

int ns_input_init(void)
{
    int err;

    ns_dev = input_allocate_device();
    if (!ns_dev)
        return -ENOMEM;

    ns_dev->name = "NanoStick";
    ns_dev->id.bustype = BUS_VIRTUAL;

    set_bit(EV_REL,  ns_dev->evbit);
    set_bit(EV_KEY,  ns_dev->evbit);
    set_bit(EV_SYN,  ns_dev->evbit);

    set_bit(REL_X, ns_dev->relbit);
    set_bit(REL_Y, ns_dev->relbit);
    set_bit(REL_WHEEL, ns_dev->relbit);

    set_bit(BTN_LEFT,  ns_dev->keybit);
    set_bit(BTN_RIGHT, ns_dev->keybit);
    set_bit(KEY_LEFTCTRL, ns_dev->keybit);

    err = input_register_device(ns_dev);
    if (err) {
        input_free_device(ns_dev);
        pr_err("[nanostick] input_register_device failed\n");
        return err;
    }

    return 0;
}

void ns_input_exit(void)
{
    if (ns_dev)
        input_unregister_device(ns_dev);
}
