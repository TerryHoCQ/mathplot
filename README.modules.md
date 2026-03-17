# C++ Modules

Around March 2026 I wanted to try to improve the 35 second build time of [a project](https://github.com/sebsjames/antpov) I was working on.

I was originally building the project with gcc-13 (and sometimes gcc-14), and linking the library [compound-ray](https://github.com/sebsjames/compound-ray)) containing CUDA and OptiX code, which was compiled with gcc-12 and CUDA's nvcc.
The build time was about 35 seconds, regardless of which source code file I changed and of how trivial that change was.

I was aware of C++ modules as a way to automate the building of an otherwise header-only codebase like mathplot.
I was interested to give it a try with the hope that it would speed up re-build times for my project.

Before starting I looked at compilers you'd need to build C++20 modules. Documentation suggested clang-18 or gcc-14 (but really 15) would be minimal requirements, along with cmake at about version 3.28 from late 2023.
With gcc-15 soon to be available in a standard Ubuntu download, and clang-18 and cmake 3.28 both available in Ubuntu 24.04, I decided that the module-supporting toolchains were available and it was worth the time making the code changes.

### Stage 1 C++20 modules

The first stage was to convert to basic C++20 modules, where I would only make modules out of my own code, and continue to use `#include <iostream>` rather than `import std;` or `import <iostream>;`.

There were several tasks

* Switch from building with cmake/Makefiles to cmake/Ninja. Add the new incantations to recognise modules files and compile these.
* Discover that the real minimum version for C++20 modules is [cmake 3.28.5](https://discourse.cmake.org/t/how-can-i-be-sure-that-a-c-20-module-is-not-re-compiled-after-a-non-module-code-change/15535). Prior to this version, your modules would *build* but a re-build would cause *all* the modules to rebuild, and thus give you no build-time speed up!
* Remove all circular dependencies from the code - there was a fundamental one in mathplot's core which required an architectural redesign (this is a real improvement).
* Switch from header-only glad to glad as a compiled library (I did consider trying to make a 'glad module' but wasn't sure it would work; the glad project doesn't have a generator for 'modularized glad')
* Make a library of the mathplot fonts to link to the executable (previously, the asm calls were header-only)
* Learn the new kind of error messages and how to understand them.
* Edit all the files to export and import modules.
* Ensure I was using modules-compatible versions of third party libraries (nlohman-json)
* Remove any non-modules compatible third party library links that I could (armadillo)
* Re-write my Bezier curve code to use `sm::mat` instead of `arma::Mat`
* Add stuff to `sm::mat` as part of the thing above (and fix some bugs)
* Report several bugs to gcc, as gcc-15 was falling over on some of my C++ once it was encapsulated in a module: [124430](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124430) [124431](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124431) [124466](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124466) [124470](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124470) [124483](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124483)

All this work took about 2 weeks.

### Stage 2 `import std;`

C++23 `import std;` is available with clang-20 and gcc-15. CMake supports it too. I figured I may as well move to `import std;` along with my C++20 `sm.` and `mplot.` modules.

This involved:

* Learning how to ensure that the toolchain would build the std.* library module. Crucially, [I had to pass the correct runtime library](https://discourse.cmake.org/t/having-difficulty-compiling-with-import-std-support-cmake-cant-build-std-cppm/15557) (libc++ for clang-20) on the cmake command line. Some lines had to change in CMakeLists.txt, too.
* Switching all use of `size_t` to the fully qualified `std::size_t` and `uint32_t` and similar from `cstdint` to `std::uint32_t` (alternatively, I could have used `import std.compat`).

I got this working over a weekend and built both maths tests and mathplot examples with `import std;`.

However, with the more complex project, I discovered a gotcha. If you link another compiled C++ library (compound-ray) then it has to be compiled with a binary compatible libc++.
With `import std;` I got best results with libc++, but compound-ray compiles with libstdc++.

I think this can be made to work, but I will either have to:

* Update the compound-ray build process to build with CUDA-13, which is clang-20 compatible
* Make the compound-ray API pure C (right now I'm using a C++ data structure to transfer data

For now, I'm sticking with regular C++20 modules, because the addition of `import std;` doesn't make that much difference to compile times.

## What I learned

* The minimum compiler versions are practically clang-20 and gcc-master (with some bugs still to be resolved, though I DID get most of the mathplot examples to build).
* Make sure to use cmake 3.28.5 or higher (I've been using the latest release, 4.2.x).
* Had I switched to clang-20 with my original header-only code, I could have improved my build times from 35 s to 20-ish seconds!
* The modules build process reduces the re-build time on the complex project to less than 10 seconds, so the work was worthwhile.
* Using C++20 modules will require that any third-party header-only code you're using is modules compatible - this just means avoiding a few small things like the `static` keyword on namespaced functions, but if your third-party library needs patching, it's an additional level of complexity.
* Using `import std;` can lead to complex linking issues, so evaluate your third party libraries carefully.

## What I will do now

For now, I'm going to work with dev/modules branches on [mathplot](https://github.com/sebsjames/mathplot/tree/dev/modules) and [maths](https://github.com/sebsjames/maths/tree/dev/modules).

I still need to convert all the mathplot examples and **mplot/**  code to modules. When it had matured enough, I'll merge it into main. C++ modules are coming for mathplot and maths...

## Matplot example profiling

Here's a profile of build times for mathplot examples. The examples built were:

```
breadcrumbs cray_eye ellipsoid geodesic graph1 grid_simple helloworld hexgrid rod rod_with_normals showcase vectorvis
```

Test machine: Rog laptop, 13th Gen Intel(R) Core(TM) i9-13980HX

All were built with this cmake line - i.e using clang20 and libc++:

```bash
CC=clang-20 CXX=clang++-20 cmake .. -G Ninja -DCMAKE_CXX_FLAGS=-stdlib=libc++
```

### With `import std;`

Building with *full* modules, including import std;

```
All examples from scratch: 42-46 sec, (13min user time)

breadcrumbs rebuild time after touch breadcrumbs.cpp (rebuilds 4 items):  5.8 s
breadcrumbs rebuild time after touch VisualModel (rebuilds 130 items):   24.0 s
```

### Without `import std;`

```
All examples from scratch: 74 s

breadcrumbs rebuild time after touch breadcrumbs.cpp:                     5.9 s
breadcrumbs rebuild time after touch VisualModel (rebuilds 11 items):    17.6 s
```

### Header only

```
All examples from scratch: 19 s

breadcrumbs rebuild time after touch breadcrumbs.cpp (rebuilds 4 items):  6.9 s
breadcrumbs rebuild time after touch VisualModel (rebuilds 130 items):    6.9 s
```

## Profiling the complex example

Test machine: Scan desktop, Intel(R) Core(TM) Ultra 9 285K

Building 'antpov'. Using clang20 across the tests with:

```bash
CC=clang-20 CXX=clang++-20 cmake .. -G Ninja -DOptiX_INSTALL_DIR=~/src/NVIDIA-OptiX-SDK-8.0.0-linux64-x86_64 -DCMAKE_CXX_FLAGS=-stdlib=libc++
```

### Without `import std;`

```
Build antpov from scratch (138 items): 26.3 s

Build after touch antpov.cpp:          8.9 s
```

### With `import std;`

Note that the final link does not complete at present, but I think these times are representative

```
Build antpov from scratch (150 items): 19.5 s

Build after touch antpov.cpp (4 items): 7.6 s
```

## Header only

```
Build antpov from scratch (2 items): 17.4 s

Build after touch antpov.cpp (2 items): 17.4 s
```