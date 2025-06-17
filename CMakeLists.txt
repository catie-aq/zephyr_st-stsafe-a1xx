if (CONFIG_LIB_STSELIB)
  zephyr_include_directories(include)
  zephyr_include_directories(include/stselib)

  FILE(GLOB_RECURSE stselib_sources include/stselib/*.c)
  zephyr_library_sources(${stselib_sources})

  zephyr_library_compile_options(
          -include ${CMAKE_CURRENT_SOURCE_DIR}/include/stse_platform_generic.h
  )

endif()
