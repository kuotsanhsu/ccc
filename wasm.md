```sh
clang++ -ibuiltininc -m32
__ptr32
```

Clang:
- https://clang.llvm.org/docs/LanguageExtensions.html#webassembly-features
- https://lld.llvm.org/WebAssembly.html
- https://clang.llvm.org/docs/ClangCommandLineReference.html
- https://github.com/llvm/llvm-project/blob/a6628e596e70bf5c31058dec582c8a7907928e98/clang/include/clang/Basic/BuiltinsWebAssembly.def#L25
- http://blog.llvm.org/2020/04/the-new-clang-extint-feature-provides.html

JS:
- https://developer.mozilla.org/en-US/docs/WebAssembly/Reference/JavaScript_interface/Memory

Blogs:
- https://mcuoneclipse.com/2024/03/13/how-to-make-sure-no-floating-point-code-is-used/
- https://milen.me/writings/swift-module-maps-vfs-overlays-header-maps/
- https://dev.to/iprosk/playing-with-low-level-memory-in-webassembly-257i
- [WebAssembly Memory Tagging](https://dl.acm.org/doi/10.1145/3733812.3765536)
- https://llvm.org/devmtg/2022-11/slides/TechTalk3-ClangClang-WebAssembly.pdf
- https://stackoverflow.com/questions/79336699/simplest-possible-memory-management-in-c-webassembly-program-how-to
- https://log.schemescape.com/posts/webassembly/c-standard-library-example.html
- https://stackoverflow.com/questions/23286440/clang-libtooling-add-new-canonical-builtin-data-type
- https://danielmangum.com/posts/every-byte-wasm-module/

Sad:
- https://news.ycombinator.com/item?id=37942040
    > Stringref is an extremely thoughtful proposal for strings in WebAssembly. It’s surprising, in a way, how thoughtful one need be about strings.
    >
    > Here is an aside, I promise it’ll be relevant. I once visited Gerry Sussman in his office, he was very busy preparing for a class and I was surprised to see that he was preparing his slides on oldschool overhead projector transparencies. “It’s because I hate computers” he said, and complained about how he could design a computer from top to bottom and all its operating system components but found any program that wasn’t emacs or a terminal frustrating and difficult and unintuitive to use (picking up and dropping his mouse to dramatic effect).
    >
    > And he said another thing, with a sigh, which has stuck with me: “Strings aren’t strings anymore.”
- https://www.infoq.com/presentations/We-Really-Dont-Know-How-To-Compute/
- https://blog.information-superhighway.net/on-the-need-for-understanding
- [L4/QNX](https://news.ycombinator.com/item?id=12539522)
