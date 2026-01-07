obj-m += fake_mouse.o
obj-m += nanostick.o
nanostick-objs := nanostick_main.o ns_input.o
nanostick-objs += ns_ldisc.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
