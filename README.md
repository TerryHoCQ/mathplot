# Mathplot: plotting and data visualization for C++

![A banner image mathplot VisualModels](https://github.com/sebsjames/mathplot/blob/main/examples/screenshots/banner2.png?raw=true)

![cmake ubuntu 24 gcc 11 build test](https://github.com/sebsjames/mathplot/actions/workflows/cmake-ubu24-gcc12.yml/badge.svg)
![cmake ubuntu 24 default (gcc 13) build test](https://github.com/sebsjames/mathplot/actions/workflows/cmake-ubuntu-2404.yml/badge.svg)
![cmake ubuntu 24 gcc 14 build test](https://github.com/sebsjames/mathplot/actions/workflows/cmake-ubu24-gcc14.yml/badge.svg)
![cmake ubuntu 24 clang 16 build test](https://github.com/sebsjames/mathplot/actions/workflows/cmake-ubu24-clang16.yml/badge.svg)
![cmake ubuntu 24 clang 17 build test](https://github.com/sebsjames/mathplot/actions/workflows/cmake-ubu24-clang16.yml/badge.svg)
![cmake ubuntu 24 clang 18 build test](https://github.com/sebsjames/mathplot/actions/workflows/cmake-ubu24-clang16.yml/badge.svg)
![cmake ubuntu 22 default (gcc 11?) build test](https://github.com/sebsjames/mathplot/actions/workflows/cmake-ubuntu-2204.yml/badge.svg)
![cmake mac 14 build test](https://github.com/sebsjames/mathplot/actions/workflows/cmake-mac-14.yml/badge.svg)
![cmake mac 15 build test](https://github.com/sebsjames/mathplot/actions/workflows/cmake-mac-15.yml/badge.svg)
![cmake windows 22 build test](https://github.com/sebsjames/mathplot/actions/workflows/cmake-windows-2022.yml/badge.svg)

**Header-only library code to visualize C++ numerical simulations using modern OpenGL.**

Mathplot is a library for drawing **3D data visualization** objects called `VisualModels`.

Mathplot is compatible with **Linux** (including **Raspberry Pi**), **Mac OS** and **Windows**.

You'll find all of the **library code** in the [**mplot**](https://github.com/sebsjames/mathplot/tree/main/mplot) directory and you can find **example code and screenshots** [here](https://github.com/sebsjames/mathplot/tree/main/examples). There is also a **template project** [that uses mathplot](https://github.com/sebsjames/mathplot_template) to help you incorporate the library into your own work.

Mathplot has a **documentation and reference** website at https://sebsjames.github.io/mathplot/.

Mathplot can be **integrated with GUI frameworks** including [Qt](https://doc.qt.io/) (see [**examples/qt/**](https://github.com/sebsjames/mathplot/tree/main/examples/qt)), [wxWidgets](https://www.wxwidgets.org/) (see [**examples/wx/**](https://github.com/sebsjames/mathplot/tree/main/examples/wx)) and [Dear ImGui](https://github.com/ocornut/imgui) (see this [ImGui example](https://github.com/sebsjames/mathplot_imgui)). Dear ImGui is the easiest way to add GUI control to your visualizations.

## Quick Start

This quick start shows dependency installation for Linux, because on this platform, it's a single call to apt (or your favourite package manager). If you're using a Mac, see [README.build.mac](https://github.com/sebsjames/mathplot/tree/main/README.build.mac.md) for help getting dependencies in place. It's [README.build.windows](https://github.com/sebsjames/mathplot/tree/main/README.build.windows.md) for Windows users. For notes on supported compilers, see [README.build.compiler](https://github.com/sebsjames/mathplot/tree/main/README.build.compiler.md)

```bash
# Install dependencies for building graph1.cpp and (almost) all the other examples (assuming Debian-like OS)
sudo apt install build-essential cmake git wget \
                 nlohmann-json3-dev librapidxml-dev \
                 freeglut3-dev libglu1-mesa-dev libxmu-dev libxi-dev \
                 libglfw3-dev libfreetype-dev libarmadillo-dev libhdf5-dev

git clone --recurse-submodules git@github.com:sebsjames/mathplot   # Get your copy of the morphologica code
cd mathplot
mkdir build         # Create a build directory
cd build
cmake ..            # Call cmake to generate the makefiles
make graph1         # Compile a single one of the examples. Add VERBOSE=1 to see the compiler commands.
./examples/graph1   # Run the program. You should see a graph of a cubic function.
# After closing the graph1 program, open its source code and modify something (see examples/graph2.cpp for ideas)
gedit ../examples/graph1.cpp
```
The program graph1.cpp is:
```c++
// Visualize a graph. Minimal example showing how a default graph appears
#include <sm/vvec> // vvec is part of Seb's maths library
#include <mplot/Visual.h>
#include <mplot/GraphVisual.h>

int main()
{
    // Set up your mplot::Visual 'scene environment'.
    mplot::Visual v(1024, 768, "Made with mplot::GraphVisual");
    // Create a new GraphVisual object with offset within the scene of 0,0,0
    auto gv = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>({0,0,0}));
    // Boilerplate bindmodel function call - do this for every model you add to a Visual
    v.bindmodel (gv);
    // Data for the x axis. sm::vvec is like std::vector, but with built-in maths methods
    sm::vvec<double> x;
    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (-0.5, 0.8, 14);
    // Set a graph up of y = x^3
    gv->setdata (x, x.pow(3));
    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();
    // Add the GraphVisual OpenGL model to the Visual scene (which takes ownership of the unique_ptr)
    v.addVisualModel (gv);
    // Render the scene on the screen until user quits with 'Ctrl-q'
    v.keepOpen();
}
```
The program generates a clean looking graph...

![Screenshot of graph1.cpp output showing a cubic function](https://github.com/sebsjames/mathplot/blob/main/examples/screenshots/graph1.png?raw=true)

...and the code is only a few lines longer than an equivalent Python program, graph1.py:
```Python
# Visualize the graph from graph1.cpp in Python
import matplotlib.pyplot as plt
import numpy as np

# Create some data for the x axis
x = np.linspace(-0.5, 0.8, 14)
# Set a graph up of y = x^3
plt.plot(x, np.power(x,3), '-o')
# Add labels
plt.title('Made with Python/numpy/matplotlib')
plt.xlabel('x')
plt.ylabel('y')
# Render the graph on the screen until user quits with 'q'
plt.show()
```
## What is Mathplot?

This header-only C++ code provides **dynamic runtime visualization**
for your programs. It was developed to visualize simulations of dynamical
systems and agent-based models in real-time.

A modern OpenGL visualization scheme called
  **[mplot::Visual](https://github.com/sebsjames/mathplot/blob/main/mplot/Visual.h)**
  provides the ability to visualise 2D and 3D graphs of surfaces,
  lines, bars, scatter plots and quiver plots with minimal processing
  overhead. Here's a [mplot::Visual
  helloworld](https://github.com/sebsjames/mathplot/blob/main/examples/helloworld.cpp)
  and [a more complete
  example](https://github.com/sebsjames/mathplot/blob/main/examples/visual.cpp). It's
  almost as easy to [draw a graph in C++ with
  mathplot](https://github.com/sebsjames/mathplot/blob/main/examples/graph1.cpp)
  as it is to do so [in
  Python](https://github.com/sebsjames/mathplot/blob/main/examples/graph1.py).

## Code documentation

See [the reference documentation website](https://sebsjames.github.io/mathplot/) for a guide to the main classes.

Mathplot code is enclosed in the **mplot** namespace. If the reference site doesn't cover it, then the header files (They're all in [mplot/](https://github.com/sebsjames/mathplot/tree/main/mplot)) contain code documentation.

You can find example programs which are compiled when you do the standard
cmake-driven build of mathplot in both the [tests/](https://github.com/sebsjames/mathplot/tree/main/tests) subdirectory
and the [examples/](https://github.com/sebsjames/mathplot/tree/main/examples) subdirectory.

There is also a template repository that demonstrates how you can create a project that *uses* mathplot: [sebsjames/mathplot_template](https://github.com/sebsjames/mathplot_template).

For more info on how to set up CMake files to build a program using mathplot (and some hints as to what you'll need to do with an alternative directed build system), see [README.cmake.md](https://github.com/sebsjames/mathplot/blob/main/README.cmake.md).

## Credits

Authorship of Mathplot code is given in each file. Copyright in
the software is owned by the authors. Refer to [morphologica](https://github.com/ABRG-Models/morphologica) for historical code authorship details (some of the code distributed here was written by contributors to morphologica). The original idea to use OpenGL to graph hexagonal grids came from [Stuart Wilson](https://github.com/stuartwilson). [Seb James](https://github.com/sebjameswml) reimplemented Stuart's hexgrid plotting code in modern OpenGL and developed morphologica into a general purpose visualization library. In 2025, [Seb](https://github.com/sebsjames) changed the name to *Mathplot* to indicate to new developers what functionality the library provides.

Mathplot is made possible by a number of third party projects whose source code is included in this repository. These include [lodepng](https://github.com/lvandeve/lodepng), [rapidxml](http://rapidxml.sourceforge.net/), [incbin](https://github.com/graphitemaster/incbin), [UniformBicone](https://github.com/wlenthe/UniformBicone), [jcvoronoi](https://github.com/JCash/voronoi) and the [HEALPix implementation from Astrometry.net](https://astrometry.net/). Thanks to the authors of these projects!

Mathplot is distributed under the terms of the Apache License, version 2 (see
[LICENSE.txt](https://github.com/sebsjames/mathplot/blob/main/LICENSE.txt)).
