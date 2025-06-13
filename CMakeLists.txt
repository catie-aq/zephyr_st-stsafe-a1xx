if (CONFIG_LIB_STSELIB)
  zephyr_include_directories(include)
  zephyr_include_directories(include/stselib)

  FILE(GLOB_RECURSE stselib_sources include/stselib/*.c)
  zephyr_library_sources(${stselib_sources})

endif()
