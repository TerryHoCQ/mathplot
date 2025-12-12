# Header installation
install(
  FILES
  compute_manager.h
  shaders.h
  texture.h
  version.h
  compute_manager_cli.h
  compute_shaderprog.h
  ssbo_nomx.h
  loadshaders_nomx.h
  loadshaders_mx.h
  util_nomx.h
  util_mx.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mplot/gl
  )
