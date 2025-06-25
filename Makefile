MAKEFLAGS += --no-builtin-rules
.DEFAULT_GOAL := all
.DELETE_ON_ERROR:
.SECONDARY: # https://stackoverflow.com/a/15191011
.SUFFIXES:

vpath %.hpp include
cpps := $(wildcard unicode/*.cpp json/*.cpp chess/*.cpp)
-include $(cpps:.cpp=.d)
CXXFLAGS += @compile_flags.txt
CPPFLAGS += -MMD -MP

%: %.o
	$(CXX) $(LDFLAGS) $(TARGET_ARCH) $(LDLIBS) -o $@ $^
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

.PHONY: clean all
clean:
	rm -fr $(tests) {unicode,json,chess}/*.{o,d,dSYM} compile_commands.json chess/main
all:

test_cpps := $(wildcard unicode/test_*.cpp json/test_*.cpp chess/test_*.cpp)
tests := $(test_cpps:.cpp=)
checks := $(tests:%=check/%)
.PHONY: check $(checks)
check: $(checks)
$(checks): check/%: %
	$^

play_chess: chess/main
	$^
