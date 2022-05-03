ifneq ($(KERNELRELEASE),)
obj-m := drivers/mfd/intel_pmt.o \
	 drivers/platform/x86/intel_pmt_class.o \
	 drivers/platform/x86/intel_pmt_telemetry.o \
	 drivers/platform/x86/intel_pmt_crashlog.o

else
KDIR ?= /lib/modules/`uname -r`/build

modules:
	$(MAKE) -C $(KDIR) M=$$PWD
	mkdir -p bin
	cp drivers/mfd/*.ko bin/
	cp drivers/platform/x86/*.ko bin/

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
	rm -rf bin/

modules_install:
	$(MAKE) -C $(KDIR) M=$$PWD modules_install

help:
	$(MAKE) -C $(KDIR) M=$$PWD help

endif
