## Algorithms

- 1974, [The Design and Analysis of Computer Algorithms](https://dl.acm.org/doi/10.5555/578775)
    - Aho, Hopcroft
- 1983, [Data structures and algorithms](https://onlinelibrary.wiley.com/doi/10.1002/net.3230150111)
    - Aho, Hopcroft, Ullman
- 1992, [Foundations of Computer Science](http://infolab.stanford.edu/~ullman/focs.html)
    - Aho, Ullman
- 1994 2e, [Concrete Mathematics: A Foundation for Computer Science](https://www-cs-faculty.stanford.edu/~knuth/gkp.html)
    - Graham, Knuth, Patashnik
- 2009, [Algorithm Design: Foundation, Analysis, and Internet Example](https://dl.acm.org/doi/10.5555/1538644)
    - Goodrich, Tamassia
- 2015, [Mathematics for Computer Science](https://people.csail.mit.edu/meyer/mcs.pdf)
    - Lehman, Leighton, Meyer
    - [MIT OpenCourseWare](https://ocw.mit.edu/courses/6-042j-mathematics-for-computer-science-spring-2015/pages/readings/)
    - [Book Review: Mathematics for Computer Science (Suggestion for MIRI Research Guide)](https://www.lesswrong.com/posts/bdvbsf8Y6FbrC2bea/book-review-mathematics-for-computer-science-suggestion-for)
- 2015 2e, [An Introduction to the Analysis of Algorithms](https://aofa.cs.princeton.edu/home/)
    - Sedgewick, Flajolet
- 2022 4e, [Introduction to Algorithms](https://mitpress.mit.edu/9780262046305/introduction-to-algorithms/)
    - Cormen, Leiserson, Rivest, Stein

### Further reading

- 1997, [Algorithms on Strings, Trees, and Sequences](https://doi.org/10.1017/CBO9780511574931)
    - Dan Gusfield
- 2019, [Mathematics and Computation: A Theory Revolutionizing Technology and Science](https://www.math.ias.edu/avi/book)
    - Avi Wigderson
- 2020, [Computational Complexity and Property Testing: On the Interplay Between Randomness and Computation](https://link.springer.com/book/10.1007/978-3-030-43662-9)
    - Oded Goldreich et al.
- 2026, [The Joy of Cryptography: An Undergraduate Course in Provable Security](https://mitpress.mit.edu/9780262049979/the-joy-of-cryptography/)
    - Mike Rosulek

## CP

- https://github.com/kuotsanhsu/cses-old
- https://github.com/kuotsanhsu/throwaway
- https://github.com/kuotsanhsu/cses
- https://github.com/kuotsanhsu/library_checker
- https://github.com/hitonanode/cplib-cpp
- https://github.com/rindag-devs/cplib
- https://news.ycombinator.com/item?id=11412583
    > Yes, in the mid nineties one of Delphi's killer features was super fast compiles. Partly possible due to a single pass compiler but it was just a very fast language to compile.
    - https://www.reddit.com/r/programming/comments/r9p4c/walter_bright_on_c_compilation_speed/
    - https://forum.dlang.org/post/nobppszcowozbztprxqx@forum.dlang.org

## Safety

- Clang, [C++ Safe Buffers](https://clang.llvm.org/docs/SafeBuffers.html)

## Allocators

- John Farrier, [Custom Allocators in C++: High Performance Memory Management](https://johnfarrier.com/custom-allocators-in-c-high-performance-memory-management/)
- John Farrier, [7 Interesting (and Powerful) Uses for C++ Iterators](https://johnfarrier.com/7-interesting-and-powerful-uses-for-c-iterators/)
- Clang, [C++ Type Aware Allocators](https://clang.llvm.org/docs/CXXTypeAwareAllocators.html)
- Open Standards, [Type-aware allocation and deallocation functions](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2719r5.html)

## Debug

- Hacker News, [rr – record and replay debugger for C/C++](https://news.ycombinator.com/item?id=41023547)

## ranges

- Stack Overflow, [Is it possible / advisable to return a range?](https://stackoverflow.com/q/67716780/16371358)
    - `std::ranges::elements_of`
    - `std::ranges::any_view`
- Stack Overflow, [Performance of simple c++20 coroutines looks bad. Is this unavoidable? Is this cost of "frame-switching"?](https://stackoverflow.com/q/77794224/16371358)
    - Type erasure performance implications.

## Packed struct

- [Packed bit fields in c structures - GCC](https://stackoverflow.com/q/25822679/16371358)
- Clang, [vector_size](https://clang.llvm.org/docs/LanguageExtensions.html#vectors-and-extended-vectors)
- Clang, [__builtin_structured_binding_size](https://clang.llvm.org/docs/LanguageExtensions.html#builtin-structured-binding-size-c)

## SIMD

- [SIMD library](https://en.cppreference.com/w/cpp/experimental/simd.html)
- Clang, [Vector Builtins](https://clang.llvm.org/docs/LanguageExtensions.html#vector-builtins)
- Clang, [Matrix Types](https://clang.llvm.org/docs/LanguageExtensions.html#matrix-types)

## Assembly

- Clang, [ASM Goto with Output Constraints](https://clang.llvm.org/docs/LanguageExtensions.html#asm-goto-with-output-constraints)
- Clang, [Constexpr strings in GNU ASM statements](https://clang.llvm.org/docs/LanguageExtensions.html#constexpr-strings-in-gnu-asm-statements)

## LLVM Blocks

- Clang, [Language Specification for Blocks](https://clang.llvm.org/docs/BlockLanguageSpec.html)
- Clang, [Block Implementation Specification](https://clang.llvm.org/docs/Block-ABI-Apple.html)
- libblocksruntime

## Clang general

- [Official doc listing](https://clang.llvm.org/docs/)
- [clang options](https://clang.llvm.org/docs/CommandGuide/clang.html)
- [diagtool](https://clang.llvm.org/docs/CommandGuide/diagtool.html)
- [vscode-lldb](https://github.com/llvm/vscode-lldb)
- [Clang-Repl](https://clang.llvm.org/docs/ClangRepl.html)
- [Clang’s refactoring engine](https://clang.llvm.org/docs/RefactoringEngine.html)

## Attributes

- `__attribute__((visibility("default")))`
- `__attribute__((used))`
- `__attribute__((weak))`

## WebAssembly

- Clang, [WebAssembly Features](https://clang.llvm.org/docs/LanguageExtensions.html#webassembly-features)

## Module map

- Clang, [API Notes: Annotations Without Modifying Headers](https://clang.llvm.org/docs/APINotes.html)
- Clang, [Modules](https://clang.llvm.org/docs/Modules.html)

## Coverage

- Clang, [Source-based Code Coverage](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html)
    - `__llvm_profile_counter_bias`
- Clang, [SanitizerCoverage](https://clang.llvm.org/docs/SanitizerCoverage.html)
    - LLVM, [The Often Misunderstood GEP Instruction](https://llvm.org/docs/GetElementPtr.html)
    - `-fsanitize-coverage=trace-cmp`
    - `-fsanitize-coverage=trace-div`
    - `-fsanitize-coverage=trace-gep`
    - `-fsanitize-coverage=trace-loads`
    - `-fsanitize-coverage=trace-stores`
    - `-fsanitize-coverage=control-flow`
    - `-fsanitize-coverage=stack-depth`
    - `-sanitizer-coverage-gated-trace-callbacks`
    - `-fsanitize-coverage=trace-pc`: syzkaller
- LLVM, [lit](https://llvm.org/docs/CommandGuide/lit.html)
- LLVM, [llvm-cov](https://llvm.org/docs/CommandGuide/llvm-cov.html)
- llvm-profdata
- VSCode extensions for showing coverage
    - [coverage-gutters](https://marketplace.visualstudio.com/items?itemName=ryanluker.vscode-coverage-gutters)
        - [Visualizing Rust Code Coverage in VS Code](https://nattrio.medium.com/visualizing-rust-code-coverage-in-vs-code-781aaf334f11)
    - [Import lcov](https://marketplace.visualstudio.com/items?itemName=gregoire.import-lcov)
    - [covertops-cmake-llvm](https://github.com/MetaBarj0/covertops-cmake-llvm)
- VSCode, [Test Coverage API](https://code.visualstudio.com/updates/v1_88#_test-coverage)
- Linux, [JITDUMP specification version 2](https://github.com/torvalds/linux/blob/master/tools/perf/Documentation/jitdump-specification.txt)

## Fuzzers

- arXiv, [Fuzzing: Art, Science, and Engineering](https://arxiv.org/pdf/1812.00140)
- [FuzzTest](https://github.com/google/fuzztest): obseletes libFuzzer
- [honggfuzz](https://github.com/google/honggfuzz)
- [What is AFL and What is it Good for?](https://github.com/google/fuzzing/blob/master/docs/afl-based-fuzzers-overview.md)
- AFL++
- Dangerous Assembly, [Coverage-Based Fuzzing](https://tetzank.github.io/posts/coverage-based-fuzzing/)
- [KCOV: code coverage for fuzzing](https://www.kernel.org/doc/html/latest/dev-tools/kcov.html)

Fuzzing services:
- OSS-Fuzz
- ClusterFuzz
- FuzzBench

### [syzkaller](https://github.com/google/syzkaller/blob/master/docs/internals.md)

- https://github.com/google/syzkaller/blob/master/docs/coverage.md
- https://github.com/google/syzkaller/blob/master/docs/linux/coverage.md
- [Syscall descriptions](https://github.com/google/syzkaller/blob/master/docs/syscall_descriptions.md)
- HN, [Why the Linux Kernel doesn't have unit tests?](https://news.ycombinator.com/item?id=33742130)
    - YouTube, [TDD, Where Did It All Go Wrong (Ian Cooper)](https://www.youtube.com/watch?v=EZ05e7EMOLM)
- HN, [Google, Xiaomi, and Huawei affected by zero-day flaw that unlocks root access](https://news.ycombinator.com/item?id=21167871)
    - YouTube, [Reflections on kernel development process, quality and testing](https://youtu.be/a2Nv-KJyqPk?t=5239)
- arXiv, [Psyzkaller: Learning from Historical and On-the-Fly Execution Data for Smarter Seed Generation in OS kernel Fuzzing](https://arxiv.org/abs/2510.08918v1)
- HN, [A Kernel Hacker Meets Fuchsia OS](https://news.ycombinator.com/item?id=31497827)
- ACM, [OZZ: Identifying Kernel Out-of-Order Concurrency Bugs with In-Vivo Memory Access Reordering](https://dl.acm.org/doi/10.1145/3694715.3695944)

https://github.com/google/syzkaller/blob/15f6fd0844e8520755be8672db814ce14cad21da/pkg/cover/backend/elf.go#L42-L49
> Normally, -fsanitize-coverage=trace-pc inserts calls to __sanitizer_cov_trace_pc() at the
> beginning of every basic block. -fsanitize-coverage=trace-cmp adds calls to other functions,
> like __sanitizer_cov_trace_cmp1() or __sanitizer_cov_trace_const_cmp4().
>
> On ARM64 there can be additional symbol names inserted by the linker. By default, BL instruction
> can only target addresses within the +/-128M range from PC. To target farther addresses, the
> ARM64 linker inserts so-called veneers that act as trampolines for functions. We count calls to
> such veneers as normal calls to __sanitizer_cov_trace_XXX.

https://github.com/google/syzkaller/blob/15f6fd0844e8520755be8672db814ce14cad21da/executor/executor_test.h#L37-L64
```c
extern "C" notrace void __sanitizer_cov_trace_pc(void)
{
	if (current_thread == nullptr || current_thread->cov.data == nullptr || current_thread->cov.collect_comps)
		return;
	uint64 pc = (uint64)__builtin_return_address(0);
	// Convert to what is_kernel_pc will accept as valid coverage;
	pc = kernel_text_start | (pc & kernel_text_mask);
	// Note: we duplicate the following code instead of using a template function
	// because it must not be instrumented which is hard to achieve for all compiler
	// if the code is in a separate function.
	if (is_kernel_64_bit) {
		uint64* start = (uint64*)current_thread->cov.data;
		uint64* end = (uint64*)current_thread->cov.data_end;
		uint64 pos = start[0];
		if (start + pos + 1 < end) {
			start[0] = pos + 1;
			start[pos + 1] = pc;
		}
	} else {
		uint32* start = (uint32*)current_thread->cov.data;
		uint32* end = (uint32*)current_thread->cov.data_end;
		uint32 pos = start[0];
		if (start + pos + 1 < end) {
			start[0] = pos + 1;
			start[pos + 1] = pc;
		}
	}
}
```

https://github.com/google/syzkaller/blob/15f6fd0844e8520755be8672db814ce14cad21da/pkg/cover/report_test.go#L185-L209
```c
#ifdef ASLR_BASE
#define _GNU_SOURCE
#endif

#include <stdio.h>

#ifdef ASLR_BASE
#include <dlfcn.h>
#include <link.h>
#include <stddef.h>

void* aslr_base() {
       struct link_map* map = NULL;
       void* handle = dlopen(NULL, RTLD_LAZY | RTLD_NOLOAD);
       if (handle != NULL) {
              dlinfo(handle, RTLD_DI_LINKMAP, &map);
              dlclose(handle);
       }
       return map ? (void *)map->l_addr : NULL;
}
#else
void* aslr_base() { return NULL; }
#endif

void __sanitizer_cov_trace_pc() { printf("%llu", (long long)(__builtin_return_address(0) - aslr_base())); }
```
