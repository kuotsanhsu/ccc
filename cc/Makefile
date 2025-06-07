.DEFAULT_GOAL := json.o
.SUFFIXES:
.SUFFIXES: .cc .o

cc_srcs := $(wildcard *.cc)
objs := $(cc_srcs:.cc=.o)
deps := $(objs:.o=.d)
-include $(deps)
CXXFLAGS += -std=c++23
CPPFLAGS += -MMD -MP -Werror -Wpointer-arith -Wimplicit-fallthrough

.PHONY: clean cleanall
clean:
	$(RM) $(objs) $(objs:.o=)
cleanall: clean
	$(RM) $(deps)
