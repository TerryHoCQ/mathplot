# Minimum compiler versions

mathplot makes extensive use of C++-20, including C++ modules.

C++ modules has been a big task for compiler and build-system developers to support.

For this reason, your compiler will need to be very up to date and may even not be able to compile mathplot.

I have had to write one or two workarounds, some of which had no effect on mathplot functionality, some of which meant I had to temporarily remove functionality for some platforms.

## Tested compiler versions

mathplot will compile with clang-20 and up.

| OS           | Compiler | Version | Result and reason                        |
| :-------:    | :------: | :-----: | ---------------------------------------- |
| Ubuntu 24.04 | g++      | 16.1    | So close! [Bug 124888](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124888) needs to be resolved |
| Ubuntu 24.04 | clang++  | 20.0    | Success |
| Ubuntu 26.04 | clang++  | 20.0    | Success |
| Ubuntu 24.04 | clang++  | 22.0    | Success |
| Windows      | VisualStudio | 17.14.36510.44 | Fails |

I had hoped that mathplot would build with gcc 15. At the moment, building on gcc 15 (on the *releases/gcc-15* branch) is blocked by the following issues:

[Bug 124470](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124470)
[Bug 124888](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124888)


## Build with clang

```bash
mkdir build_clang
cd build_clang
CC=clang-20 CXX=clang++-20 cmake .. -GNinja
ninja
```

## Build with gcc

```bash
mkdir build_gcc
cd build_gcc
CC=/opt/gcc-master/bin/gcc CXX=/opt/gcc-master/bin/g++ cmake .. -GNinja
ninja
```
