zephyr_library_named(stsafe_driver)
zephyr_library_sources_ifdef(CONFIG_STSAFE_DRIVER stsafe.c)
zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/../../include)
