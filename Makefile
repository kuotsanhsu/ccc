MAKEFLAGS += --jobs=$(shell nproc || sysctl -n hw.ncpu || 1) --output-sync=target
MAKEFLAGS += --no-print-directory
.DEFAULT_GOAL := all

recursive_goals := all test clean cleanall
.PHONY: $(recursive_goals)
$(recursive_goals):
	$(MAKE) -C src $@
