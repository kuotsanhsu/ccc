.DEFAULT_GOAL := all
.SUFFIXES:
.SUFFIXES: .c .o

srcs := $(wildcard *.c)
objs := $(srcs:.c=.o)
deps := $(srcs:.c=.d)
-include $(deps)
CFLAGS += -MMD -MP

.PHONY: all test
all: $(objs)
test: $(patsubst %.c,exec_%,$(wildcard test_*.c))
exec_%: %
	exec ./$<
test_unicode: test_unicode.o unicode.o
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: clean cleanall
clean:
	$(RM) $(objs) $(objs:.o=)
cleanall: clean
	$(RM) $(deps)
