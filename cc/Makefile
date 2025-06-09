.DEFAULT_GOAL := test_json_syntax
.SUFFIXES:
.SUFFIXES: .cc .o

cc_srcs := $(wildcard *.cc)
objs := $(cc_srcs:.cc=.o)
deps := $(objs:.o=.d)
-include $(deps)
CXXFLAGS += -std=c++23
CPPFLAGS += -MMD -MP -Werror -Wpointer-arith -Wimplicit-fallthrough
LDFLAGS += -flto=thin

test_json_syntax: test_json_syntax.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: clean cleanall
clean:
	$(RM) $(objs) $(objs:.o=)
cleanall: clean
	$(RM) $(deps)
