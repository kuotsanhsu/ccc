.DEFAULT_GOAL := all

recursive_goals := all test clean cleanall
.PHONY: $(recursive_goals)
$(recursive_goals):
	$(MAKE) -C src $@
