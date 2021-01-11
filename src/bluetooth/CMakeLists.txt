
add_library(bt_lib STATIC
        connection_manager/connection_manager.cpp
        connection_manager/connection_manager.hpp
        services/ess/environmental_sensing_service.cpp services/ess/environmental_sensing_service.hpp connection_manager/connection_handler.cpp connection_manager/connection_handler.hpp)

target_include_directories(bt_lib   PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(bt_lib  PRIVATE
        zephyr_interface
        utils_lib
        )