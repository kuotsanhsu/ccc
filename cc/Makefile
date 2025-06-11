.DEFAULT_GOAL := test_json
.SUFFIXES:
.SUFFIXES: .cc .o

cc_srcs := $(wildcard *.cc)
objs := $(cc_srcs:.cc=.o)
deps := $(objs:.o=.d)
-include $(deps)
CXXFLAGS += -std=c++23 -fno-exceptions
CPPFLAGS += -MMD -MP -Werror -Wpointer-arith -Wimplicit-fallthrough
LDFLAGS += -flto=thin

# test_unicode: test_unicode.o
# test_json: test_json.o
# test_unicode test_json_syntax:
# 	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: clean cleanall
clean:
	$(RM) $(objs) $(objs:.o=)
cleanall: clean
	$(RM) $(deps)
