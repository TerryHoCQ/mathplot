# Modules build times

The examples built were:

breadcrumbs
cray_eye
ellipsoid
geodesic
graph1
grid_simple
helloworld
hexgrid
rod
rod_with_normals
showcase
vectorvis

Test machine: Rog laptop, 13th Gen Intel(R) Core(TM) i9-13980HX

All were built with this cmake line:

```bash
CC=clang-20 CXX=clang++-20 cmake .. -G Ninja -DCMAKE_CXX_FLAGS=-stdlib=libc++
```

## With `import std;`

Building with *full* modules, including import std;

clang20: 42-46 sec, (13min user time)

breadcrumbs rebuild time after touch breadcrumbs.cpp (rebuilds 4 items):  5.8 s
breadcrumbs rebuild time after touch VisualModel (rebuilds 130 items):   24.0 s

clang21: Can't test yet, need libc++ built from clang21

## Without `import std;`

clang20: 74 s

breadcrumbs rebuild time after touch breadcrumbs.cpp:                     5.9 s
breadcrumbs rebuild time after touch VisualModel (rebuilds 11 items):    17.6 s

## Header only

clang20: 19 s

breadcrumbs rebuild time after touch breadcrumbs.cpp (rebuilds 4 items):  6.9 s
breadcrumbs rebuild time after touch VisualModel (rebuilds 130 items):    6.9 s
