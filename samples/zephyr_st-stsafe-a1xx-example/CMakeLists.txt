# Copyright (c) 2025 CATIE
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(zest_security_secureelement_example LANGUAGES C)

zephyr_include_directories(include)

file(GLOB_RECURSE app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
