ARCH ?= mips
CROSS_COMPILE ?= mips-linux-gnu-

KDIR ?= $(PWD)/../../../os/kernel/kernel-4.4.94
MODULE_NAME := ltr-311als

all: modules

.PHONY: modules clean

$(MODULE_NAME)-objs := ltr311als.o
obj-m := $(MODULE_NAME).o

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

clean:
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order

release:
	mkdir -p ../modules/${ZRT_ENV_TOOL_CHAIN}/$(subst _,.,${ZRT_ENV_LINUX_VERSION})
	cp ${MODULE_NAME}.ko ../modules/${ZRT_ENV_TOOL_CHAIN}/$(subst _,.,${ZRT_ENV_LINUX_VERSION})