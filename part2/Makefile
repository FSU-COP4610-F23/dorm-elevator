CC=/usr/bin/gcc-11 
obj-m += my_timer.o

my_timer-objs := src/my_timer.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load: 
	sudo insmod my_timer.ko

unload: 
	sudo rmmod my_timer.ko
