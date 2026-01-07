```sh
cd build

clang++ -fsanitize=fuzzer fuzz_target.cpp # libfuzzer_not_found.log

sudo port install clang-21 # clang-install.log

/opt/local/bin/clang++-mp-21 -fsanitize=fuzzer -o fuzz_target_01 ../fuzz_target.cpp
# ld: warning: ignoring duplicate libraries: '-lc++'
./fuzz_target_01 # fuzz_target_01.log
cat crash-c0a0ad26a634840c67a210fefdda76577b03a111 # "Hi!"

/opt/local/bin/clang++-mp-21 -fsanitize=fuzzer,address -o fuzz_target_02 ../fuzz_target.cpp
./fuzz_target_02 # fuzz_target_02.log

/opt/local/bin/clang++-mp-21 -g -O1 -fsanitize=fuzzer,address -o fuzz_target_03 ../fuzz_target.cpp
./fuzz_target_03 # fuzz_target_03.log
```

libfuzzer_not_found.log:
```log
ld: warning: ignoring duplicate libraries: '-lc++'
ld: library '/Library/Developer/CommandLineTools/usr/lib/clang/17/lib/darwin/libclang_rt.fuzzer_osx.a' not found
clang++: error: linker command failed with exit code 1 (use -v to see invocation)
```

clang-install.log:
```log
db48 has the following notes:
  The Java and Tcl bindings are now provided by the db48-java and
  db48-tcl subports.
libomp has the following notes:
  To use this OpenMP library:
    * For clang-3.8+, or clang-3.7 with +openmp variant:
      add "-fopenmp" during compilation / linking.
    * For clang-3.7 without +openmp variant, use:
      "-I/opt/local/include/libomp -L/opt/local/lib/libomp -fopenmp"
```

fuzz_target_01.log:
```log
INFO: Running with entropic power schedule (0xFF, 100).
INFO: Seed: 2588917943
INFO: Loaded 1 modules   (9 inline 8-bit counters): 9 [0x102e50000, 0x102e50009), 
INFO: Loaded 1 PC tables (9 PCs): 9 [0x102e50010,0x102e500a0), 
INFO: -max_len is not provided; libFuzzer will not generate inputs larger than 4096 bytes
INFO: A corpus is not provided, starting from an empty corpus
#2      INITED cov: 2 ft: 2 corp: 1/1b exec/s: 0 rss: 26Mb
#92     NEW    cov: 3 ft: 3 corp: 2/2b lim: 4 exec/s: 0 rss: 26Mb L: 1/1 MS: 5 ShuffleBytes-InsertByte-EraseBytes-CopyPart-ChangeByte-
#105    NEW    cov: 4 ft: 4 corp: 3/5b lim: 4 exec/s: 0 rss: 26Mb L: 3/3 MS: 3 CopyPart-ChangeByte-CrossOver-
#132    REDUCE cov: 4 ft: 4 corp: 3/4b lim: 4 exec/s: 0 rss: 26Mb L: 2/2 MS: 2 CrossOver-EraseBytes-
#14419  NEW    cov: 5 ft: 5 corp: 4/38b lim: 142 exec/s: 0 rss: 27Mb L: 34/34 MS: 2 InsertByte-InsertRepeatedBytes-
#14420  REDUCE cov: 5 ft: 5 corp: 4/33b lim: 142 exec/s: 0 rss: 27Mb L: 29/29 MS: 1 EraseBytes-
#14449  REDUCE cov: 5 ft: 5 corp: 4/23b lim: 142 exec/s: 0 rss: 27Mb L: 19/19 MS: 4 InsertByte-InsertByte-ChangeBinInt-EraseBytes-
#14469  REDUCE cov: 5 ft: 5 corp: 4/15b lim: 142 exec/s: 0 rss: 27Mb L: 11/11 MS: 5 ShuffleBytes-InsertByte-CMP-ShuffleBytes-EraseBytes- DE: "\015\000\000\000\000\000\000\000"-
#14491  REDUCE cov: 5 ft: 5 corp: 4/11b lim: 142 exec/s: 0 rss: 27Mb L: 7/7 MS: 2 PersAutoDict-EraseBytes- DE: "\015\000\000\000\000\000\000\000"-
#14509  REDUCE cov: 5 ft: 5 corp: 4/10b lim: 142 exec/s: 0 rss: 27Mb L: 6/6 MS: 3 ChangeBit-ChangeByte-EraseBytes-
#14850  REDUCE cov: 5 ft: 5 corp: 4/8b lim: 142 exec/s: 0 rss: 27Mb L: 4/4 MS: 1 EraseBytes-
#14883  REDUCE cov: 5 ft: 5 corp: 4/7b lim: 142 exec/s: 0 rss: 27Mb L: 3/3 MS: 3 CMP-ChangeBit-EraseBytes- DE: "\001P"-
#15215  REDUCE cov: 6 ft: 6 corp: 5/9b lim: 142 exec/s: 0 rss: 27Mb L: 2/3 MS: 2 CopyPart-EraseBytes-
#27211  REDUCE cov: 7 ft: 7 corp: 6/13b lim: 261 exec/s: 0 rss: 27Mb L: 4/4 MS: 1 InsertByte-
Assertion failed: (false), function LLVMFuzzerTestOneInput, file fuzz_target.cpp, line 11.
==6829== ERROR: libFuzzer: deadly signal
    #0 0x0001031ababc in __sanitizer_print_stack_trace+0x10 (libclang_rt.ubsan_osx_dynamic.dylib:arm64+0x7abc)
    #1 0x000102e42144 in fuzzer::PrintStackTrace()+0x2c (fuzz_target:arm64+0x100036144)
    #2 0x000102e251ac in fuzzer::Fuzzer::CrashCallback()+0x54 (fuzz_target:arm64+0x1000191ac)
    #3 0x00019a637740 in _sigtramp+0x34 (libsystem_platform.dylib:arm64+0x3740)
    #4 0x00019a62d884 in pthread_kill+0x124 (libsystem_pthread.dylib:arm64+0x6884)
    #5 0x00019a53284c in abort+0x78 (libsystem_c.dylib:arm64+0x7984c)
    #6 0x00019a531a80 in __assert_rtn+0x118 (libsystem_c.dylib:arm64+0x78a80)
    #7 0x000102e0cac8 in LLVMFuzzerTestOneInput+0x1d0 (fuzz_target:arm64+0x100000ac8)
    #8 0x000102e26850 in fuzzer::Fuzzer::ExecuteCallback(unsigned char const*, unsigned long)+0x134 (fuzz_target:arm64+0x10001a850)
    #9 0x000102e25fd8 in fuzzer::Fuzzer::RunOne(unsigned char const*, unsigned long, bool, fuzzer::InputInfo*, bool, bool*)+0x3c (fuzz_target:arm64+0x100019fd8)
    #10 0x000102e27b24 in fuzzer::Fuzzer::MutateAndTestOne()+0x1e8 (fuzz_target:arm64+0x10001bb24)
    #11 0x000102e2884c in fuzzer::Fuzzer::Loop(std::__1::vector<fuzzer::SizedFile, std::__1::allocator<fuzzer::SizedFile>>&)+0x3c4 (fuzz_target:arm64+0x10001c84c)
    #12 0x000102e18a18 in fuzzer::FuzzerDriver(int*, char***, int (*)(unsigned char const*, unsigned long))+0x1ed8 (fuzz_target:arm64+0x10000ca18)
    #13 0x000102e42c80 in main+0x24 (fuzz_target:arm64+0x100036c80)
    #14 0x00019a265d50  (<unknown module>)

NOTE: libFuzzer has rudimentary signal handlers.
      Combine libFuzzer with AddressSanitizer or similar for better crash reports.
SUMMARY: libFuzzer: deadly signal
MS: 1 EraseBytes-; base unit: 0b86b12c4a4c29eab4f058541e63a22fb62b138a
0x48,0x69,0x21,
Hi!
artifact_prefix='./'; Test unit written to ./crash-c0a0ad26a634840c67a210fefdda76577b03a111
Base64: SGkh
```

fuzz_target_02.log:
```log
INFO: Running with entropic power schedule (0xFF, 100).
INFO: Seed: 3041951366
INFO: Loaded 1 modules   (9 inline 8-bit counters): 9 [0x1002f4000, 0x1002f4009), 
INFO: Loaded 1 PC tables (9 PCs): 9 [0x1002f4010,0x1002f40a0), 
INFO: -max_len is not provided; libFuzzer will not generate inputs larger than 4096 bytes
INFO: A corpus is not provided, starting from an empty corpus
#2      INITED cov: 2 ft: 2 corp: 1/1b exec/s: 0 rss: 34Mb
#923    NEW    cov: 3 ft: 3 corp: 2/4b lim: 11 exec/s: 0 rss: 35Mb L: 3/3 MS: 1 CMP- DE: "H\000"-
#1018   REDUCE cov: 3 ft: 3 corp: 2/3b lim: 11 exec/s: 0 rss: 35Mb L: 2/2 MS: 5 CrossOver-CopyPart-PersAutoDict-ChangeByte-EraseBytes- DE: "H\000"-
#1034   NEW    cov: 4 ft: 4 corp: 3/4b lim: 11 exec/s: 0 rss: 35Mb L: 1/2 MS: 1 CrossOver-
#10586  REDUCE cov: 5 ft: 5 corp: 4/6b lim: 104 exec/s: 0 rss: 35Mb L: 2/2 MS: 2 ChangeByte-ChangeBit-
#10652  NEW    cov: 6 ft: 6 corp: 5/10b lim: 104 exec/s: 0 rss: 35Mb L: 4/4 MS: 1 CrossOver-
#10674  REDUCE cov: 6 ft: 6 corp: 5/9b lim: 104 exec/s: 0 rss: 35Mb L: 3/3 MS: 2 EraseBytes-InsertByte-
Assertion failed: (false), function LLVMFuzzerTestOneInput, file fuzz_target.cpp, line 11.
==7562== ERROR: libFuzzer: deadly signal
    #0 0x0001007e3530 in __sanitizer_print_stack_trace+0x28 (libclang_rt.asan_osx_dynamic.dylib:arm64+0x5f530)
    #1 0x0001002e63d4 in fuzzer::PrintStackTrace()+0x2c (fuzz_target_02:arm64+0x1000363d4)
    #2 0x0001002c943c in fuzzer::Fuzzer::CrashCallback()+0x54 (fuzz_target_02:arm64+0x10001943c)
    #3 0x00019a637740 in _sigtramp+0x34 (libsystem_platform.dylib:arm64+0x3740)
    #4 0x00019a62d884 in pthread_kill+0x124 (libsystem_pthread.dylib:arm64+0x6884)
    #5 0x00019a53284c in abort+0x78 (libsystem_c.dylib:arm64+0x7984c)
    #6 0x00019a531a80 in __assert_rtn+0x118 (libsystem_c.dylib:arm64+0x78a80)
    #7 0x0001002b0cb8 in LLVMFuzzerTestOneInput+0x2d0 (fuzz_target_02:arm64+0x100000cb8)
    #8 0x0001002caae0 in fuzzer::Fuzzer::ExecuteCallback(unsigned char const*, unsigned long)+0x134 (fuzz_target_02:arm64+0x10001aae0)
    #9 0x0001002ca268 in fuzzer::Fuzzer::RunOne(unsigned char const*, unsigned long, bool, fuzzer::InputInfo*, bool, bool*)+0x3c (fuzz_target_02:arm64+0x10001a268)
    #10 0x0001002cbdb4 in fuzzer::Fuzzer::MutateAndTestOne()+0x1e8 (fuzz_target_02:arm64+0x10001bdb4)
    #11 0x0001002ccadc in fuzzer::Fuzzer::Loop(std::__1::vector<fuzzer::SizedFile, std::__1::allocator<fuzzer::SizedFile>>&)+0x3c4 (fuzz_target_02:arm64+0x10001cadc)
    #12 0x0001002bcca8 in fuzzer::FuzzerDriver(int*, char***, int (*)(unsigned char const*, unsigned long))+0x1ed8 (fuzz_target_02:arm64+0x10000cca8)
    #13 0x0001002e6f10 in main+0x24 (fuzz_target_02:arm64+0x100036f10)
    #14 0x00019a265d50  (<unknown module>)

NOTE: libFuzzer has rudimentary signal handlers.
      Combine libFuzzer with AddressSanitizer or similar for better crash reports.
SUMMARY: libFuzzer: deadly signal
MS: 1 InsertByte-; base unit: 94dd9e08c129c785f7f256e82fbe0a30e6d1ae40
0x48,0x69,0x21,
Hi!
artifact_prefix='./'; Test unit written to ./crash-c0a0ad26a634840c67a210fefdda76577b03a111
Base64: SGkh
```

fuzz_target_03.log:
```log
INFO: Running with entropic power schedule (0xFF, 100).
INFO: Seed: 3292749577
INFO: Loaded 1 modules   (9 inline 8-bit counters): 9 [0x100694000, 0x100694009), 
INFO: Loaded 1 PC tables (9 PCs): 9 [0x100694010,0x1006940a0), 
INFO: -max_len is not provided; libFuzzer will not generate inputs larger than 4096 bytes
INFO: A corpus is not provided, starting from an empty corpus
#2      INITED cov: 2 ft: 2 corp: 1/1b exec/s: 0 rss: 34Mb
#1904   NEW    cov: 3 ft: 3 corp: 2/15b lim: 21 exec/s: 0 rss: 35Mb L: 14/14 MS: 2 ChangeBit-InsertRepeatedBytes-
#1912   REDUCE cov: 3 ft: 3 corp: 2/13b lim: 21 exec/s: 0 rss: 35Mb L: 12/12 MS: 3 InsertRepeatedBytes-EraseBytes-EraseBytes-
#1913   REDUCE cov: 3 ft: 3 corp: 2/11b lim: 21 exec/s: 0 rss: 35Mb L: 10/10 MS: 1 EraseBytes-
#1922   REDUCE cov: 3 ft: 3 corp: 2/3b lim: 21 exec/s: 0 rss: 35Mb L: 2/2 MS: 4 ChangeBinInt-CrossOver-EraseBytes-EraseBytes-
#1963   REDUCE cov: 4 ft: 4 corp: 3/4b lim: 21 exec/s: 0 rss: 35Mb L: 1/2 MS: 1 EraseBytes-
#2793   NEW    cov: 5 ft: 5 corp: 4/10b lim: 29 exec/s: 0 rss: 35Mb L: 6/6 MS: 5 CrossOver-CrossOver-InsertByte-CrossOver-InsertByte-
#2992   REDUCE cov: 5 ft: 5 corp: 4/7b lim: 29 exec/s: 0 rss: 35Mb L: 3/3 MS: 4 ShuffleBytes-CrossOver-ShuffleBytes-EraseBytes-
#3023   REDUCE cov: 6 ft: 6 corp: 5/9b lim: 29 exec/s: 0 rss: 35Mb L: 2/3 MS: 1 EraseBytes-
#13231  NEW    cov: 7 ft: 7 corp: 6/14b lim: 128 exec/s: 0 rss: 35Mb L: 5/5 MS: 3 ChangeByte-CrossOver-InsertByte-
Assertion failed: (false), function LLVMFuzzerTestOneInput, file fuzz_target.cpp, line 10.
==7936== ERROR: libFuzzer: deadly signal
    #0 0x000100d47530 in __sanitizer_print_stack_trace+0x28 (libclang_rt.asan_osx_dynamic.dylib:arm64+0x5f530)
    #1 0x0001006862bc in fuzzer::PrintStackTrace() FuzzerUtil.cpp:210
    #2 0x000100669324 in fuzzer::Fuzzer::CrashCallback() FuzzerLoop.cpp:231
    #3 0x00019a637740 in _sigtramp+0x34 (libsystem_platform.dylib:arm64+0x3740)
    #4 0x00019a62d884 in pthread_kill+0x124 (libsystem_pthread.dylib:arm64+0x6884)
    #5 0x00019a53284c in abort+0x78 (libsystem_c.dylib:arm64+0x7984c)
    #6 0x00019a531a80 in __assert_rtn+0x118 (libsystem_c.dylib:arm64+0x78a80)
    #7 0x000100650bc8 in LLVMFuzzerTestOneInput fuzz_target.cpp:10
    #8 0x00010066a9c8 in fuzzer::Fuzzer::ExecuteCallback(unsigned char const*, unsigned long) FuzzerLoop.cpp:619
    #9 0x00010066a150 in fuzzer::Fuzzer::RunOne(unsigned char const*, unsigned long, bool, fuzzer::InputInfo*, bool, bool*) FuzzerLoop.cpp:516
    #10 0x00010066bc9c in fuzzer::Fuzzer::MutateAndTestOne() FuzzerLoop.cpp:765
    #11 0x00010066c9c4 in fuzzer::Fuzzer::Loop(std::__1::vector<fuzzer::SizedFile, std::__1::allocator<fuzzer::SizedFile>>&) FuzzerLoop.cpp:910
    #12 0x00010065cb90 in fuzzer::FuzzerDriver(int*, char***, int (*)(unsigned char const*, unsigned long)) FuzzerDriver.cpp:916
    #13 0x000100686df8 in main FuzzerMain.cpp:20
    #14 0x00019a265d50  (<unknown module>)

NOTE: libFuzzer has rudimentary signal handlers.
      Combine libFuzzer with AddressSanitizer or similar for better crash reports.
SUMMARY: libFuzzer: deadly signal
MS: 1 EraseBytes-; base unit: cdf38f3af521a1b3a056fcc400aa9b84be8cd0b4
0x48,0x69,0x21,
Hi!
artifact_prefix='./'; Test unit written to ./crash-c0a0ad26a634840c67a210fefdda76577b03a111
Base64: SGkh
```
