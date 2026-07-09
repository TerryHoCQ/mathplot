# Header installation
install(
  FILES
  compute_manager.h
  shaders.h
  texture.h
  version.cppm
  compute_manager_cli.h
  compute_shaderprog.h
  ssbo_nomx.h
  ssbo_mx.h
  loadshaders_nomx.h
  loadshaders_mx.h
  util_nomx.h
  util_mx.cppm
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mplot/gl
  )
