
add_library(bt_lib STATIC
        connection_manager/connection_manager.cpp
        connection_manager/connection_manager.hpp
        services/ess/environmental_sensing_service.cpp
        services/ess/environmental_sensing_service.hpp
        connection_manager/connection_handler.cpp
        connection_manager/connection_handler.hpp
        adv_btn/advertise_button.cpp
        adv_btn/advertise_button.hpp
        shared/msg_passer.cpp
        shared/msg_passer.hpp
        )

target_include_directories(bt_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>

        PRIVATE
        $<TARGET_PROPERTY:utils_lib,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:sensor_lib,INTERFACE_INCLUDE_DIRECTORIES>
        )

target_link_libraries(bt_lib
        PRIVATE
        zephyr_interface
        kernel
        utils_lib
        sensor_lib
        etl
        )
