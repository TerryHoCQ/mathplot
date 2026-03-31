include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(a a.cpp)
target_sources(a PUBLIC FILE_SET CXX_MODULES BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES
  ${PROJECT_SOURCE_DIR}/bugrep/B.cppm
)

set(CMAP_MODULES
  ${SM_VEC_MODULES}
  ${SM_FLAGS_MODULES}
  ${SM_CRC32_MODULES}
  ${PROJECT_SOURCE_DIR}/mplot/tools.cppm
  ${PROJECT_SOURCE_DIR}/mplot/ColourMap.cppm
  ${PROJECT_SOURCE_DIR}/mplot/colourmaps_cet.cppm
  ${PROJECT_SOURCE_DIR}/mplot/colourmaps_crameri.cppm
  ${PROJECT_SOURCE_DIR}/mplot/ColourMap_Lists.cppm
)
list(REMOVE_DUPLICATES CMAP_MODULES)

add_executable(cmap cmap.cpp)
target_sources(cmap PUBLIC FILE_SET CXX_MODULES BASE_DIRS ${PROJECT_SOURCE_DIR} FILES ${CMAP_MODULES})
