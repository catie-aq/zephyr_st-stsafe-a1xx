if (CONFIG_LIB_STSELIB)
  zephyr_include_directories(stselib)
 
  FILE(GLOB_RECURSE stselib_sources *.c)
  zephyr_library_sources(${stselib_sources})
  
endif()
