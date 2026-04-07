# Minimum compiler versions

mathplot makes extensive use of C++-20, including C++ modules.

## Tested compiler versions

mathplot will compile with clang-20 and up or gcc 16. At the time of writing, gcc 16 is not released and so it is necessary to compile gcc from the master branch.

| OS           | Compiler | Version | Result and reason                        |
| :-------:    | :------: | :-----: | ---------------------------------------- |
| Ubuntu 24.04 | g++      | 16.0    | Ok |
| Ubuntu 24.04 | clang++  | 20.0    | Ok |

I had hoped that mathplot would build with gcc 15, because this will be the default compiler on Ubuntu 25. At the moment, building on gcc 15 (on the *releases/gcc-15* branch) is blocked by the following issues:

[Bug 124470](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124470)


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
