# Building your code with mathplot

These instructions will be a little out of date now we build with C++ 20 modules.

## Dependencies

First, ensure you have the necessary dependencies installed. Classes in mathplot use nlohmann-json (submoduled), rapidxml, OpenGL, Freetype, glfw3 and HDF5.
You won't necessarily need all of these; it depends on which classes you will use (see [here](https://github.com/sebsjames/mathplot/blob/main/README.coding.md#linking-a-mathplot-program) for details).
For most visualisation, you only need OpenGL, Freetype and glfw3.
Platform-specific instructions can be found in the files [README.build.linux.md](https://github.com/sebsjames/mathplot/blob/main/README.build.linux.md), [README.build.mac.md](https://github.com/sebsjames/mathplot/blob/main/README.build.mac.md) and [README.build.windows.md](https://github.com/sebsjames/mathplot/blob/main/README.build.windows.md).

## Three necessities to build

Regardless of which build process you use (plain makefiles, autotools,
CMake or whatever), to build a program against mathplot, you need
to tell it: **1**) What compiler flags to add to the
compiler command line, including a directive to say where the fonts
that mathplot will compile into your binaries (if you're using
mplot::Visual) are located. **2**) Where the mathplot headers (in *mplot/*) are
to be found. **3**) which modules to build and which libraries to link to.

While you can install mathplot headers (and fonts) into a chosen
location (/usr/local by default) we recommend that you just clone a
copy of the mathplot repository into the base of your own source tree.

## Building with CMake

If you're using cmake, then this is what you add to your client code's
CMakeLists.txt:

### 1) Compiler flags

This piece of boiler-plate cmake will get you started with a sensible
set of compiler flags for mathplot:

```cmake
# mathplot uses c++-20 language features
set(CMAKE_CXX_STANDARD 20)

# Add the host definition to CXXFLAGS along with other switches,
# depending on OS/Compiler and your needs/preferences
if (APPLE)
  set(WARNING_FLAGS "-Wall -Wfatal-errors")
else() # assume g++ (or a gcc/g++ mimic like Clang)
  set(WARNING_FLAGS "-Wall -Wfatal-errors -Wno-unused-result -Wno-unknown-pragmas")
endif()
set(CMAKE_CXX_FLAGS "-g -O3 ${WARNING_FLAGS}")

# Add OpenMP flags here, if necessary
find_package(OpenMP)
if(OpenMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Additional GL compiler flags.
set(OpenGL_GL_PREFERENCE "GLVND")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES")
if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL_SILENCE_DEPRECATION")
endif()

# Tell the program where the mathplot fonts are, to compile them into the binary
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMPLOT_FONTS_DIR=\"\\\"${PROJECT_SOURCE_DIR}/fonts\\\"\"")
```

The last flag (**MPLOT_FONTS_DIR**) helps your compiler to copy in the
fonts that mplot::Visual needs.  If you are working with and
'installed' mathplot change this to:

```cmake
# Tell the program where the mplot fonts are. Again, assuming you installed mathplot in /usr/local:
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMPLOT_FONTS_DIR=\"\\\"/usr/local/share/fonts\\\"\"")
```

### 2) Include directories for headers

```cmake
# Find the libraries which will be needed
find_package(HDF5 REQUIRED)        # Only required if you used sm::hdfdata
find_package(OpenGL REQUIRED)      # This, glfw3, Freetype and nlohmann-json are required for mplot::Visual
find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)

# Define collections of includes for the dependencies
set(MPLOT_INC_CORE ${HDF5_INCLUDE_DIR})
set(MPLOT_INC_GL ${OPENGL_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS})
include_directories(${MPLOT_INC_CORE} ${MPLOT_INC_GL})

# Assuming that you installed mathplot 'in-tree'
set(MPLOT_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/mathplot" CACHE PATH "The path to mathplot")
include_directories(BEFORE ${MPLOT_INCLUDE_PATH})         # Allows mplot/Header.h to be found
```
If you are working with a system-installed mathplot (with /usr/local/include/mplot or similar), then replace the last section with:
```cmake
# MPLOT_INCLUDE_PATH is set to the location at which the header directory 'mplot' is found. For 'Installed mathplot':
set(MPLOT_INCLUDE_PATH /usr/local CACHE PATH "The path to the mathplot headers (e.g. /usr/local/include or \$HOME/usr/include)")
include_directories(BEFORE ${MPLOT_INCLUDE_PATH}/include)       # Allows mplot/Header.h to be found
```

### 3) Links to dynamically linked libraries

Mathplot makes use of a number of libraries. Depending on which
classes you use from mathplot, you'll need to link to some or all
of these:

```cmake
set(MPLOT_LIBS_CORE ${HDF5_C_LIBRARIES})
set(MPLOT_LIBS_GL OpenGL::GL Freetype::Freetype glfw)
target_link_libraries(myprogtarget ${MPLOT_LIBS_CORE} ${MPLOT_LIBS_GL})
```

### Example build files

Need link to example repos here.