# Copyright (c) 2025 CATIE
# SPDX-License-Identifier: Apache-2.0

if (CONFIG_LIB_STSELIB)
  zephyr_include_directories(include)
  zephyr_include_directories(include/stselib)
  zephyr_include_directories(include/helpers)

  FILE(GLOB_RECURSE stselib_sources include/stselib/*.c)
  FILE(GLOB_RECURSE helpers_sources include/helpers/*.c)

  zephyr_library_sources(${stselib_sources} ${helpers_sources})

  zephyr_compile_options(
          -include ${CMAKE_CURRENT_SOURCE_DIR}/include/stse_platform_generic.h
  )

endif()
