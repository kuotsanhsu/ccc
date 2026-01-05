# Compile for x86_64-linux-gnu on macOS

- Mentor Graphics, [Which targets does Clang support?](https://llvm.org/devmtg/2014-04/PDFs/LightningTalks/2014-3-31_ClangTargetSupport_LighteningTalk.pdf)
- Gentoo Linux, [Project:Prefix](https://wiki.gentoo.org/wiki/Project:Prefix)
- Clang, [Assembling a Complete Toolchain](https://clang.llvm.org/docs/Toolchain.html)
- Clang, [Clang command line argument reference](https://clang.llvm.org/docs/ClangCommandLineReference.html)
- Clang, [Configuration files](https://clang.llvm.org/docs/UsersManual.html#configuration-files)
- GitHub, [tpoechtrager/osxcross](https://github.com/tpoechtrager/osxcross)
- GitHub, [messense/homebrew-macos-cross-toolchains](https://github.com/messense/homebrew-macos-cross-toolchains)

On macOS, `clang -print-targets` yields:
```txt
  Registered Targets:
    aarch64    - AArch64 (little endian)
    aarch64_32 - AArch64 (little endian ILP32)
    aarch64_be - AArch64 (big endian)
    arm        - ARM
    arm64      - ARM64 (little endian)
    arm64_32   - ARM64 (little endian ILP32)
    armeb      - ARM (big endian)
    thumb      - Thumb
    thumbeb    - Thumb (big endian)
    x86        - 32-bit X86: Pentium-Pro and above
    x86-64     - 64-bit X86: EM64T and AMD64
```

## Install Linux toolchain

```sh
sudo port selfupdate

port search x86_64 # x86_64-linux-binutils
sudo port install x86_64-linux-binutils
port contents x86_64-linux-binutils | grep include # EMPTY
sudo port uninstall x86_64-linux-binutils
port contents x86_64-linux-binutils
# Error: port contents failed: Registry error: x86_64-linux-binutils not registered as installed.

port contents x86_64-elf-gcc
# Error: port contents failed: Registry error: x86_64-elf-gcc not registered as installed.

sudo port install x86_64-elf-gcc
port contents x86_64-elf-gcc | grep include
```

x86_64-elf-gcc is freestanding. The following things are missing:
```log
/opt/local/x86_64-elf/bin/ld: cannot find Scrt1.o: No such file or directory
/opt/local/x86_64-elf/bin/ld: cannot find crti.o: No such file or directory
/opt/local/x86_64-elf/bin/ld: cannot find crtbeginS.o: No such file or directory
/opt/local/x86_64-elf/bin/ld: cannot find -lgcc_s
/opt/local/x86_64-elf/bin/ld: cannot find -lgcc_s
/opt/local/x86_64-elf/bin/ld: cannot find crtendS.o: No such file or directory
/opt/local/x86_64-elf/bin/ld: cannot find crtn.o: No such file or directory
```

## Homebrew

- Download pkg from https://github.com/Homebrew/brew/releases/latest

```sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
# Homebrew installation log
cat /Users/gordonh/.zprofile # Homebrew not found. Good.

alias brew=/opt/homebrew/bin/brew
brew tap messense/macos-cross-toolchains
brew install x86_64-unknown-linux-gnu
brew list x86_64-unknown-linux-gnu # /opt/homebrew/Cellar/x86_64-unknown-linux-gnu/13.3.0/toolchain
brew --prefix x86_64-unknown-linux-gnu # /opt/homebrew/opt/x86_64-unknown-linux-gnu
unalias brew

clang++ @cross_flags.txt hello.cpp
scp a.out novum.nvidia.com:~
ssh novum.nvidia.com ./a.out # hello
```

compile_flags.txt:
```
--target=x86_64-unknown-linux-gnu
--gcc-toolchain=/opt/homebrew/opt/x86_64-unknown-linux-gnu/toolchain
--sysroot=/opt/homebrew/opt/x86_64-unknown-linux-gnu/toolchain/x86_64-unknown-linux-gnu/sysroot
-fuse-ld=/opt/local/x86_64-elf/bin/ld
```

Homebrew installation log:
```log
==> Installation successful!

==> Homebrew has enabled anonymous aggregate formulae and cask analytics.
Read the analytics documentation (and how to opt-out) here:
  https://docs.brew.sh/Analytics
No analytics data has been sent yet (nor will any be during this install run).

==> Homebrew is run entirely by unpaid volunteers. Please consider donating:
  https://github.com/Homebrew/brew#donations

==> Next steps:
- Run these commands in your terminal to add Homebrew to your PATH:
    echo >> /Users/gordonh/.zprofile
    echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> /Users/gordonh/.zprofile
    eval "$(/opt/homebrew/bin/brew shellenv)"
- Run brew help to get started
- Further documentation:
    https://docs.brew.sh
```

## aarch64-linux-gnu with Apple Container

- https://github.com/GoogleContainerTools/distroless/blob/main/cc/README.md
- https://github.com/GoogleContainerTools/distroless/blob/main/examples/cc/Dockerfile
- Apple container cannot mount single file:
    - https://github.com/apple/containerization/issues/79
- [Add libc++ to distroless/cc](https://github.com/GoogleContainerTools/distroless/issues/394)

```sh
target='aarch64-unknown-linux-gnu'
alias brew='/opt/homebrew/bin/brew'
brew tap messense/macos-cross-toolchains
brew install "$target"
toolchain="$(brew --prefix "$target")/toolchain" # /opt/homebrew/opt/aarch64-unknown-linux-gnu
unalias brew

cat > "${target}.cfg" <<EOF
--target=$target
--gcc-toolchain=$toolchain
--sysroot=$toolchain/$target/sysroot
--prefix=$toolchain/$target/bin
EOF

echo "/build/" >> .gitignore
mkdir build
clang++ @compile_flags.txt @aarch64-unknown-linux-gnu.cfg -o build/hello hello.cpp
"$toolchain/$target/bin/readelf" -d build/hello | head

container run -it \
    --mount type=bind,source=build,target=/build,readonly \
    gcr.io/distroless/cc /build/hello
    # libstdc++ version issue

container run -it --name hello \
    --mount type=bind,source=build,target=/build,readonly \
    debian /build/hello
container start -ia hello

# container run -it --rm -v "$PWD/build:/build:ro" debian /build/hello
# container run -it --name hello -v "$PWD/build:/build:ro" debian /build/hello
# container start -ai hello
```
