ifneq ($(KERNELRELEASE),)
intel_vsec-y := vsec.o
pmt_class-y := pmt/class.o
pmt_telemetry-y := pmt/telemetry.o
pmt_crashlog-y := pmt/crashlog.o
obj-m := intel_vsec.o pmt_class.o pmt_telemetry.o pmt_crashlog.o

INCLUDES = -I$(src)/../../../../include
ccflags-y := $(INCLUDES)

ccflags-y += -DDISTRO_NAME="$(shell cat /etc/os-release | grep PRETTY_NAME | sed 's/PRETTY_NAME="//;s/"//' | sed 's/ /-/g')" \
			 -DKERNEL_VERSION_NAME="$(shell echo $(KDIR) | cut -d'/' -f4)" \
			 -DVSEC_BACKPORT_VERSION="$(MODULE_VERSION)"

else
KDIR ?= /lib/modules/`uname -r`/build

modules:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

modules_install:
	$(MAKE) -C $(KDIR) M=$$PWD modules_install

help:
	$(MAKE) -C $(KDIR) M=$$PWD help

endif
