obj-m = semap.o
semap-y = main.o

all:
	make -C /lib/modules/`uname -r`/build EXTRA_CFLAGS=-I$(shell pwd)/include M=`pwd`

clean:
	rm -rf *.o *.ko *.mod.c .*.cmd modules.order Module.symvers .cache.mk .tmp_versions main.o.*