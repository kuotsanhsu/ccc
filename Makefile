.DEFAULT_GOAL := all
.DELETE_ON_ERROR:
.SECONDARY: # https://stackoverflow.com/a/15191011
.SUFFIXES:

vpath %.hpp include
cpps := $(wildcard unicode/*.cpp json/*.cpp)
-include $(cpps:.cpp=.d)
CXXFLAGS += @compile_flags.txt
CPPFLAGS += -MMD -MP

%: %.o
	$(CXX) $(LDFLAGS) $(TARGET_ARCH) $(LDLIBS) -o $@ $^
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

test_cpps := $(wildcard unicode/test_*.cpp json/test_*.cpp)
tests := $(test_cpps:.cpp=)
exec_tests := $(tests:%=exec/%)
.PHONY: clean all test $(exec_tests)
clean:
	rm -fr $(tests) {unicode,json}/*.{o,d,dSYM} compile_commands.json
all:
test: $(exec_tests)
$(exec_tests): exec/%: %
	$^
