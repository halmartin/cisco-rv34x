TOP_DIR := $(shell pwd)

obj-m += sbr_cdev.o

obj-m += nbvpn.o
nbvpn-y := \
	nbvpn_core.o \
	nbvpn_ctrl.o

all:
	$(MAKE) -C $(KERNEL_SOURCE) ARCH=arm SUBDIRS="$(TOP_DIR)" modules

clean:
	make ARCH=arm -C $(KERNEL_SOURCE) M=$(PWD) clean

