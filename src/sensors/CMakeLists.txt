
add_library(sensor_lib STATIC
        interface/humidity/humidity_sensor.cpp
        interface/humidity/humidity_sensor.hpp

        interface/moisture/moisture_sensor.cpp
        interface/moisture/moisture_sensor.hpp

        interface/temperature/temperature_sensor.cpp
        interface/temperature/temperature_sensor.hpp

        bme280/bme_280.cpp bme280/bme_280.hpp

        )

target_include_directories(sensor_lib PUBLIC
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>

        PRIVATE
        $<TARGET_PROPERTY:utils_lib,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:bt_lib,INTERFACE_INCLUDE_DIRECTORIES>
        )

target_link_libraries(sensor_lib
        PRIVATE
        zephyr_interface
        kernel
        bt_lib
        utils_lib
        etl
        )
