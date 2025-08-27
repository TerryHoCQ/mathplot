# CTests

# All #includes in test programs have to be #include "mplot/header.h"
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

if(ARMADILLO_FOUND)

  # Test reading trial.svg
  set(TARGETTEST3 testreadcurves)
  set(SOURCETEST3 testreadcurves.cpp)
  add_executable(${TARGETTEST3} ${SOURCETEST3})
  target_link_libraries(${TARGETTEST3} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
  add_test(testreadcurves ${TARGETTEST3})

  # Test reading whiskerbarrels_withcentres.svg
  set(TARGETTEST31 testreadcurves_circles)
  set(SOURCETEST31 testreadcurves_circles.cpp)
  add_executable(${TARGETTEST31} ${SOURCETEST31})
  target_link_libraries(${TARGETTEST31} ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES})
  add_test(testreadcurves_circles ${TARGETTEST31})

endif(ARMADILLO_FOUND)

if(${glfw3_FOUND})
  if(ARMADILLO_FOUND)
    # Test hexgrid3 (hexgrid2 with visualisation)
    add_executable(testhexgrid3 testhexgrid3.cpp)
    target_link_libraries(testhexgrid3 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype)

    if(HDF5_FOUND)
      # Test hexgridsave (HexGrid's save feature)
      add_executable(testhexgridsave testhexgridsave.cpp)
      target_link_libraries(testhexgridsave ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${HDF5_C_LIBRARIES} OpenGL::GL glfw Freetype::Freetype)
    endif(HDF5_FOUND)

    add_executable(testbighexgrid testbighexgrid.cpp)
    target_link_libraries(testbighexgrid OpenGL::GL glfw Freetype::Freetype)

    add_executable(testbezderiv3 testbezderiv3.cpp)
    target_compile_definitions(testbezderiv3 PUBLIC FLT=double)
    target_link_libraries(testbezderiv3 ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} OpenGL::GL glfw Freetype::Freetype)
    add_test(testbezderiv3 testbezderiv3)

  endif(ARMADILLO_FOUND)

  add_executable(testVisCoordArrows testVisCoordArrows.cpp)
  target_link_libraries(testVisCoordArrows OpenGL::GL glfw Freetype::Freetype)

  add_executable(testVisRemoveModel testVisRemoveModel.cpp)
  target_link_libraries(testVisRemoveModel OpenGL::GL glfw Freetype::Freetype)

  if(ARMADILLO_FOUND)
    # Test elliptical HexGrid code (visualized with mplot::Visual)
    add_executable(test_ellipseboundary test_ellipseboundary.cpp)
    target_link_libraries(test_ellipseboundary OpenGL::GL glfw Freetype::Freetype)

    # Test circular HexGrid code
    add_executable(test_circleboundary test_circleboundary.cpp)
    target_link_libraries(test_circleboundary OpenGL::GL glfw Freetype::Freetype)
  endif(ARMADILLO_FOUND)

endif()

# Test the colour mapping
add_executable(testColourMap testColourMap.cpp)
add_test(testColourMap testColourMap)

add_executable(testrgbhsv testrgbhsv.cpp)
add_test(testrgbhsv testrgbhsv)

# mplot::tools
add_executable(testTools testTools.cpp)
add_test(testTools testTools)

add_executable(testloadpng testloadpng.cpp)
add_test(testloadpng testloadpng)

add_executable(testgraphnumberformat testgraphnumberformat.cpp)
add_test(testgraphnumberformat testgraphnumberformat)

add_executable(testgraphnumberformat_ss testgraphnumberformat.cpp)
target_compile_definitions(testgraphnumberformat_ss PUBLIC FORCE_NON_FORMAT)
add_test(testgraphnumberformat_ss testgraphnumberformat_ss)

add_executable(testmaketicks testmaketicks.cpp)
add_test(testmaketicks testmaketicks)

add_executable(testmakeformatticks testmakeformatticks.cpp)
add_test(testmakeformatticks testmakeformatticks)
