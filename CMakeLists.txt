# Copyright (c) 2025 CATIE
# SPDX-License-Identifier: Apache-2.0

if (CONFIG_LIB_STSELIB)
  set(STSELIB_DIR ${WEST_TOPDIR}/modules/lib/stselib)

  zephyr_include_directories(
    include
    ${STSELIB_DIR}
    ${WEST_TOPDIR}/modules/lib
  )

  if (CONFIG_STSE_USE_HOST_SESSION)
    target_include_directories(${ZEPHYR_CURRENT_LIBRARY} PUBLIC
      ${WEST_TOPDIR}/modules/crypto/mbedtls/include
    )
  endif()

  file(GLOB_RECURSE stselib_sources ${STSELIB_DIR}/*.c)

  zephyr_library_sources(
    ${stselib_sources}

    src/port/crc16.c
    src/port/crypto.c
    src/port/i2c.c
    src/port/services.c
  )

  zephyr_library_sources_ifdef(CONFIG_STSE_USE_HOST_SESSION
    src/port/cmac.c
  )

  zephyr_compile_options(
    -include ${CMAKE_CURRENT_SOURCE_DIR}/include/stse_platform_generic.h
  )
endif()
