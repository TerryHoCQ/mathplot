include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(a a.cpp)
target_sources(a PUBLIC FILE_SET CXX_MODULES BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES
  ${PROJECT_SOURCE_DIR}/bugrep/B.cppm
)
