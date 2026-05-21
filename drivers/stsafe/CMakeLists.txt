# Copyright (c) 2026 CATIE
# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(stsafe_driver)

zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)
zephyr_library_sources(stsafe.c)

if(CONFIG_LIB_STSELIB)
  set(STSELIB_DIR ${WEST_TOPDIR}/modules/lib/stselib)

  zephyr_include_directories(
    ${STSELIB_DIR}
    ${WEST_TOPDIR}/modules/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
  )

  if(CONFIG_STSE_USE_HOST_SESSION)
    zephyr_include_directories(
      ${WEST_TOPDIR}/modules/crypto/mbedtls/include
    )
  endif()

  file(GLOB_RECURSE stselib_sources ${STSELIB_DIR}/*.c)
  zephyr_library_sources(${stselib_sources})

  zephyr_library_sources(
    platform/crc16.c
    platform/crypto.c
    platform/i2c.c
    platform/services.c
  )

  zephyr_library_sources_ifdef(CONFIG_STSE_USE_HOST_SESSION
    platform/aes.c
    platform/cmac.c
    platform/key_store.c
  )

  zephyr_library_compile_options(
    -include ${CMAKE_CURRENT_SOURCE_DIR}/platform/stse_platform_generic.h
  )

endif() # CONFIG_LIB_STSELIB
