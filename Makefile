DRIVER_DIR = drivers/platform/x86/intel

.PHONY: modules clean modules_install help

OTHER_TARGETS := modules_install help

include/generated_osv_version.h:
	scripts/generate_osv_version_h.sh > $@

modules: include/generated_osv_version.h
	$(MAKE) -C $(DRIVER_DIR)
	mkdir -p bin
	cp drivers/platform/x86/intel/*.ko bin/

clean:
	$(MAKE) -C $(DRIVER_DIR) clean
	rm -rf bin/ *.out

$(OTHER_TARGETS):
	$(MAKE) -C $(DRIVER_DIR) $(MAKECMDGOALS)
