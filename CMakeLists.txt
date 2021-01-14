# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

set(ENV{BOARD} nrf52840dk_nrf52840)
set(ENV{BOARD_DIR} $ENV{ZEPHYR_BASE}/boards/arm/nrf52840dk_nrf52840)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(emesh_sensor
        LANGUAGES C CXX ASM)

enable_language(CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

target_sources(app PRIVATE
  src/main.cpp
)

add_subdirectory(third_party/zpp)
add_subdirectory(third_party/etl)
target_include_directories(etl INTERFACE third_party/etl_profile)

#zephyr_library_include_directories(${ZEPHYR_BASE}/samples/bluetooth)
add_subdirectory(src/sensors)
add_subdirectory(src/bluetooth)
add_subdirectory(src/utils)

target_link_libraries(app PRIVATE
        sensor_lib
        utils_lib
        bt_lib
        etl
        )