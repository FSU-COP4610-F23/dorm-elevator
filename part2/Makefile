CC=/usr/bin/gcc-11 # Replace with the path to the correct GCC version
obj-m += my_timer.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean


