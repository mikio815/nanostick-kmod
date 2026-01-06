#ifndef NS_ACTION_H
#define NS_ACTION_H

#include <linux/input.h>

enum ns_action_type {
    NS_ACT_MOVE,
    NS_ACT_SCROLL,
    NS_ACT_CLICK,
    NS_ACT_ZOOM,
};

struct ns_action {
    enum ns_action_type type;
    int x;
    int y;
    int wheel;
    int button;
    int pressed;
};

static inline void ns_emit_action(struct input_dev *dev, const struct ns_action *a)
{
    switch (a->type) {
    case NS_ACT_MOVE:
        if (a->x)
            input_report_rel(dev, REL_X, a->x);
        if (a->y)
            input_report_rel(dev, REL_Y, a->y);
        input_sync(dev);
        break;
    case NS_ACT_SCROLL:
        if (a->wheel)
            input_report_rel(dev, REL_WHEEL, a->wheel);
        input_sync(dev);
        break;
    case NS_ACT_CLICK:
        input_report_key(dev, a->button, a->pressed);
        input_sync(dev);
        break;
    case NS_ACT_ZOOM:
        input_report_key(dev, KEY_LEFTCTRL, 1);
        input_report_rel(dev, REL_WHEEL, a->wheel);
        input_sync(dev);
        input_report_key(dev, KEY_LEFTCTRL, 0);
        input_sync(dev);
        break;
    }
}

#endif
