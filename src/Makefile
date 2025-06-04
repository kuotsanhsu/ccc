.DEFAULT_GOAL := all
.SUFFIXES:
.SUFFIXES: .c .cc .o

c_srcs := $(wildcard *.c)
cc_srcs := $(wildcard *.cc)
objs := $(c_srcs:.c=.o) $(cc_srcs:.cc=.o)
deps := $(objs:.o=.d)
-include $(deps)
CFLAGS += -std=c23
CXXFLAGS += -std=c++23
CPPFLAGS += -MMD -MP -Werror -Wpointer-arith -Wimplicit-fallthrough

execs := $(addprefix exec_,$(filter test_%,$(objs:.o=)))
.PHONY: all test $(execs)
all: $(objs)
test: $(execs)
$(execs): exec_%: %
	exec ./$<
test_unicode: test_unicode.o unicode.o
test_json: test_json.o json.o unicode.o
test_unicode test_json:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@
test_unicode_more: test_unicode_more.o unicode.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: clean cleanall
clean:
	$(RM) $(objs) $(objs:.o=)
cleanall: clean
	$(RM) $(deps)
