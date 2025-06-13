if (CONFIG_LIB_STSELIB)
  zephyr_include_directories(stselib)
  zephyr_include_directories(include)

  FILE(GLOB_RECURSE stselib_sources *.c)
  zephyr_library_sources(${stselib_sources})

endif()
