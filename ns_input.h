#ifndef NS_INPUT_H
#define NS_INPUT_H

int ns_input_init(void);
void ns_input_exit(void);
struct input_dev *ns_input_get_dev(void);

#endif
