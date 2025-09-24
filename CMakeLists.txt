# Copyright (c) 2025 CATIE
# SPDX-License-Identifier: Apache-2.0

if (CONFIG_LIB_STSELIB)
  set(STSELIB_DIR ${WEST_TOPDIR}/modules/lib/stselib)

  zephyr_include_directories(
    include
    ${STSELIB_DIR}
    ${WEST_TOPDIR}/modules/lib
  )

  file(GLOB_RECURSE stselib_sources ${STSELIB_DIR}/*.c)

  zephyr_library_sources(
    ${stselib_sources}

    src/port/crc16.c
    src/port/crypto.c
    src/port/i2c.c
    src/port/services.c
  )

  zephyr_compile_options(
    -include ${CMAKE_CURRENT_SOURCE_DIR}/include/stse_platform_generic.h
  )
endif()
