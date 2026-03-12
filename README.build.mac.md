# Build mathplot on Apple Mac

You don't need to *build* mathplot to use the headers, but
you *will* need to install the dependencies.

The cmake-driven mathplot build process compiles a set of test and
example programs which require all of the dependencies to be met.

Programs that ```#include``` mathplot headers will also need to link to
some or all of those dependencies. Finally, you'll need the cmake
program and a C++ compiler which can compile c++-17 code.

## Installation dependencies for Mac

You will need XCode from the App Store. If you just installed XCode,
then you'll need to agree to its licence terms. To do this, run

```
sudo xcodebuild -license
```

scroll through the legalese and type 'agree' (assuming that you
do). Also, do make sure to run XCode at least once from the launcher
as this will prompt it to download and install some additional
components. Finally, it seems to be necessary to "install command line
tools" to get a working compiler. To do so (at least on MacOS
Catalina):

```
xcode-select --install
```

Installation of most of the other dependencies can be achieved using Mac
ports (Option 1, below). This does lead to the installation of a great deal of
additional software, some of which can conflict with Mac system
software (that's libiconv, in particular). However, a clean install of
Mac ports will successfully install the dependencies for
mathplot. Another managed option to to use the brew system (Option 3, below).

I'd advise you to use Option 1: Mac Ports only if you *already* use Mac Ports, and Option 3 if you already use brew.
Otherwise, prefer Option 2: Manual dependency builds.

### Option 1: Mac Ports

Install Mac ports, following the instructions on
http://www.macports.org/. This will guide you to install the XCode
command line tools, then install the Mac ports installation package.

Finally, use Mac ports to install the rest of the dependencies:

```sh
sudo port install cmake
```

*Be aware that if you have conflicting versions of any of the
 libraries in another location (such as /usr/local), you may run into
 problems during the build.*

Now skip Option 2 and go to **Common manual dependency builds** to
compile glfw3 by hand.

### Option 2: Manual dependency builds

It's much cleaner to build each of the dependencies by hand. That
means first installing cmake, which I do with a binary package from
https://cmake.org/download/, and then compiling hdf5.

After downloading and installing cmake using the MacOS installer, I
add these lines to ~/.zprofile so that I can type cmake at the terminal:

```sh
# Add cmake bin directory to path so you can type 'cmake'
export PATH="/Applications/CMake.app/Contents/bin:${PATH}"
```

### Option 3: brew

The mathplot github actions for mac runners use brew to install
dependencies. The command for the basic dependencies is

```sh
brew install libomp glfw hdf5 # nlohmann-json
# nlohmann-json was removed as I have to bundle a very up to date version for C++ modules support
```

#### HDF5

Hierarchical data format is a standard format for saving binary
data. `sm::hdfdata` wraps the HDF5 API and hence HDF5 is a required
dependency to build all of the mathplot tests and is required if
you are going to use `sm::hdfdata`. Build version 1.10.x.

```sh
mkdir -p ~/src
cd ~/src
tar xvf path/to/downloaded/hdf5-1.10.7.tar.gz
cd hdf5-1.10.7
mkdir build
cd build
cmake ..
make
sudo make install
```

#### Qt5

If present, the Qt5 components Core, Gui and Widgets components are used to compile some example Qt mathplot programs. I've not tested this on Mac.


### Common manual dependency builds

Whether or not you used mac ports to install hdf5, glfw3 also needs to be built separately (I've not investigated
whether there is a mac ports version of glfw).

#### glfw3

The modern OpenGL code in mathplot requires the GL-window managing
library GLFW3. Compile it like this:

```
cd ~/src
git clone https://github.com/glfw/glfw.git
cd glfw
mkdir build
cd build
cmake ..
make
sudo make install
```

## Build mathplot on Mac

To build mathplot itself, it's the usual CMake process:

```sh
cd ~/src
git clone https://github.com/sebsjames/mathplot.git
cd mathplot
mkdir build
cd build
# If you have OpenMP, you can add -DCOMPILE_WITH_OPENMP=1
cmake ..
make -j$(nproc)
sudo make install
```

## Tests

To run the tests you have to build them by changing your cmake command in the recipe above to:
```sh
cmake .. -DBUILD_TESTS=ON
```

To run the test suite, use the `ctest` command in the build directory.
