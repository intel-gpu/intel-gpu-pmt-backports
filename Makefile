DRIVER_DIR = drivers/platform/x86/intel

.PHONY: modules clean modules_install help

OTHER_TARGETS := modules_install help

modules:
	$(MAKE) -C $(DRIVER_DIR)
	mkdir -p bin
	cp drivers/platform/x86/intel/*.ko bin/

clean:
	$(MAKE) -C $(DRIVER_DIR) clean
	rm -rf bin/ *.out

$(OTHER_TARGETS):
	$(MAKE) -C $(DRIVER_DIR) $(MAKECMDGOALS)
