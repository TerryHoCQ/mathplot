# Building with mathplot on GNU/Linux

You don't need to *build* mathplot to use the headers, but
you *will* need to install the dependencies.

The cmake-driven mathplot build process compiles a set of test and
example programs which require all of the dependencies to be met.

Programs that ```#include``` mathplot headers will also need to link to
some or all of those dependencies. Finally, you'll need the cmake
program and a C++ compiler which can compile c++-20 code.

## *Required*: Install dependencies

mathplot code depends on OpenGL, Freetype and glfw3. Armadillo and HDF5 are optional dependencies which you may need. Armadillo is required if you use the `sm::bezcurve` class or any of the classes `sm::hexgrid`/`mplot::ReadCurve`/`sm::cartgri`d (which all use `sm::bezcurvepath` and hence `sm::bezcurve`). HDF5 is required if you use the `sm::hdfdata` wrapper class, or if you want to compile `sm::hexgrid`/`sm::cartgrid` with built-in `save()` and `load()` functions.

### Package-managed dependencies for Ubuntu/Debian

To install the visualization dependencies on Ubuntu or Debian Linux:

```sh
sudo apt install build-essential cmake git \
                 nlohmann-json3-dev librapidxml-dev \
                 freeglut3-dev libglu1-mesa-dev libxmu-dev libxi-dev \
                 libglfw3-dev libfreetype-dev

```
For the optional dependencies it's:
```sh
sudo apt install libarmadillo-dev libhdf5-dev qtcreator qtbase5-dev libwxgtk3.2-dev libegl-dev libgbm-dev
```
* Armadillo. Only required if you use the ```sm::bezcurve``` class.
* HDF5 library. Required if you use the wrapper class ```sm::hdfdata``` or any of the classes that make use of `sm::hdfdata` (```sm::hexgrid```,```sm::cartgrid```,```sm::anneal```). Their tests and examples should all compile if the libraries are detected and be omitted if not.
* Qt library. Installing qtcreator will bring in the Qt5 libraries that are used to compile some Qt-mathplot example programs. It almost certainly possible to install *only* the Qt5 Core, Gui and Widgets libraries, but that hasn't been verified. On recent Ubuntu systems, you may well need qtbase5-dev to get the cmake scripts to `find_package(Qt5...)`.
* WxWindows. libwxgtk3.2-dev (you'll need Ubuntu 23.04+) will enable the compilation of mathplot-wxWidgets example programs.
* EGL. Required to build GLES applications that are compatible with Raspberry Pi 4 and 5.
* GBM. Required only for window-less OpenGL compute compilations. Currently that's one example program only.

### Package-managed dependencies for Arch Linux

On Arch Linux, all required dependencies except Armadillo are available in the official repository. They can be installed as follows:

```shell
sudo pacman -S vtk lapack blas freeglut glfw-wayland nlohmann-json
# Optional:
sudo pacman -S hdf5
```

**Note:** Specify `glfw-x11` instead of `glfw-wayland` if you use X.org.

Then, optionally, install [Armadillo](https://aur.archlinux.org/packages/armadillo/) from AUR.

## *Optional*: Build mathplot examples or tests

To build the mathplot example programs, it's the usual CMake process:

```sh
cd ~/src
git clone https://github.com/sebsjames/mathplot.git
cd mathplot
mkdir build
cd build
cmake ..
make -j$(nproc)
# I usually place the mathplot directory inside the code repository I'm working
# on, I call this 'in-tree mathplot', but you can also have the headers in
# /usr/local/include (control location with the usual CMAKE_INSTALL_PREFIX) if you install:
# sudo make install
```
### Building test programs (or NOT building the examples)

By default, the example programs are built with the call to `make`, but unit test programs are not. To build test programs, and control whether example programs are compiled, use the cmake flags `BUILD_TESTS` and `BUILD_EXAMPLES`, changing your cmake line to:
```sh
cmake .. -DBUILD_TESTS=ON -DBUILD_EXAMPLES=OFF # Build tests but not examples
# ...etc
```

There is also the flag `BUILD_OPTIONAL_EXAMPLES` to enable any programs that use additional dependencies that can't be tested for in the base `CMakeLists.txt`. Currently, that's just one example that uses libgbm.

If you need to build the test programs with a specific compiler, such
as g++-11 or clang, then you just change the cmake call in the recipe
above. It becomes:

```sh
CXX=g++-11 cmake .. -DBUILD_TESTS=ON
```
To run the test suite, use the `ctest` command in the build directory or `make test`.

### Build the client code

See the top level README for a quick description of how to include mathplot in your client code and [README.cmake.md] for more information.

### Building some of the dependencies manually

Only necessary if the package managed libs don't work.

#### HDF5 library build

You will also need HDF5 installed on your system. There _is_ an HDF5 package for Ubuntu, but I couldn't get the mathplot cmake build process to find it nicely, so I compiled my own version of HDF5 and installed in /usr/local. To do what I did, download HDF5 (https://portal.hdfgroup.org/display/support/Downloads), and do a compile and install like this:

```sh
mkdir -p ~/src
cd ~/src
cp path/to/hdf5-1.10.x.tar.gz ./
gunzip hdf5-1.10.x.tar.gz
tar xvf hdf5-1.10.x.tar
cd hdf5-1.10.x
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
```

#### glfw3 library build

The modern OpenGL code in Visual/HexGridVisual requires the library GLFW3.

It's possible to apt install glfw on recent versions of Ubuntu. Doing so
will install libglfw.a. These build instructions install libglfw3.a (into
/usr/local/lib/).

```
sudo apt install libxinerama-dev libxrandr-dev libxcursor-dev
cd ~/src
git clone https://github.com/glfw/glfw.git
cd glfw
mkdir build
cd build
cmake ..
make
sudo make install
```
