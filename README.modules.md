# Modules build times

## Matplot examples

The examples built were:

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

All examples from scratch: 42-46 sec, (13min user time)

breadcrumbs rebuild time after touch breadcrumbs.cpp (rebuilds 4 items):  5.8 s
breadcrumbs rebuild time after touch VisualModel (rebuilds 130 items):   24.0 s

### Without `import std;`

All examples from scratch: 74 s

breadcrumbs rebuild time after touch breadcrumbs.cpp:                     5.9 s
breadcrumbs rebuild time after touch VisualModel (rebuilds 11 items):    17.6 s

### Header only

All examples from scratch: 19 s

breadcrumbs rebuild time after touch breadcrumbs.cpp (rebuilds 4 items):  6.9 s
breadcrumbs rebuild time after touch VisualModel (rebuilds 130 items):    6.9 s

## Complex example

Test machine: Scan desktop, Intel(R) Core(TM) Ultra 9 285K

Building 'antpov'. Using clang20 across the tests with:

```bash
CC=clang-20 CXX=clang++-20 cmake .. -G Ninja -DOptiX_INSTALL_DIR=~/src/NVIDIA-OptiX-SDK-8.0.0-linux64-x86_64 -DCMAKE_CXX_FLAGS=-stdlib=libc++
```

### Without `import std;`

Build antpov from scratch (138 items): 26.3 s

Build after touch antpov.cpp: 8.9 s

### With `import std;`

Note that the final link does not complete at present, but I think these times are representative

Build antpov from scratch (150 items): 19.5 s

Build after touch antpov.cpp (4 items): 7.6 s

## Header only

Build antpov from scratch (2 items): 17.4 s

Build after touch antpov.cpp (2 items): 17.4 s
