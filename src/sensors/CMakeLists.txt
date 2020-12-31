
add_library(sensor_lib STATIC
        interface/humidity/humidity_sensor.cpp
        interface/humidity/humidity_sensor.hpp

        interface/moisture/moisture_sensor.cpp
        interface/moisture/moisture_sensor.hpp

        interface/temperature/temperature_sensor.cpp
        interface/temperature/temperature_sensor.hpp
        )

target_include_directories(sensor_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/interface
        )