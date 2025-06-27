MAKEFLAGS += --no-builtin-rules
.DEFAULT_GOAL := check
.DELETE_ON_ERROR:
.SECONDARY: # https://stackoverflow.com/a/15191011
.SUFFIXES:

cpps := $(wildcard *.cpp)
-include $(cpps:.cpp=.d)
CXXFLAGS += @compile_flags.txt
CPPFLAGS += -MMD -MP

%: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $(LDLIBS) -o $@ $^

# random_03 large_cycle_00
tests := \
	example_00 \
	random_00 random_01 random_02 random_04 \
	max_random_00 max_random_01 max_random_02 max_random_03 max_random_04
.PHONY: clean check $(tests)
clean:
	rm -fr scc *.{o,d,dSYM}
check: $(tests)
$(tests): %: scc testcases/%.in testcases/%.out
	diff --brief <(./$< < testcases/$*.in) testcases/$*.out
testcases/%.in:
	curl --silent --fail --location --create-dirs --output $@ \
		"https://lc-api.hotman78.com/api/graph/scc/in/$*.in"
testcases/%.out:
	curl --silent --fail --location --create-dirs --output $@ \
		"https://lc-api.hotman78.com/api/graph/scc/out/$*.out"
