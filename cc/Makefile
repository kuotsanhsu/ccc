.DEFAULT_GOAL := test_unicode
.SUFFIXES:
.SUFFIXES: .cc .o

cc_srcs := $(wildcard *.cc)
objs := $(cc_srcs:.cc=.o)
deps := $(objs:.o=.d)
-include $(deps)
CXXFLAGS += -std=c++23
CPPFLAGS += -MMD -MP -Werror -Wpointer-arith -Wimplicit-fallthrough
LDFLAGS += -flto=thin

test_unicode: test_unicode.o unicode.o
test_json_syntax: test_json_syntax.o
test_unicode test_json_syntax:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: clean cleanall
clean:
	$(RM) $(objs) $(objs:.o=)
cleanall: clean
	$(RM) $(deps)
