obj-m += elevator.o

elevator-objs := src/elevator.o src/proc_file.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load: 
	sudo insmod elevator.ko

unload:
	sudo rmmod elevator

watch: 
	watch -n1 cat /proc/elevator

