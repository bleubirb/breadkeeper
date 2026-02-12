# makefile for i2c linux driver, i don't think this should work for now (not tested)
ifneq ($(KERNELRELEASE),)
	obj-m := i2c.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
modules:
	echo $(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	$(MAKE) -C $(KERNELDIR) M=$(PWD) LDDINC==$(PWD)/include modules
endif

all default: modules
install: modules_install
	$(MAKE) -C $(KERNELDIR) M=$(PWD) LDDINC==$(PWD)/include modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean