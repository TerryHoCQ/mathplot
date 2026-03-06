install(FILES gl.h gl_mx.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mplot/glad)

# Build glad library here
include_directories(BEFORE ${PROJECT_SOURCE_DIR})
add_library(glad SHARED gl.c)
