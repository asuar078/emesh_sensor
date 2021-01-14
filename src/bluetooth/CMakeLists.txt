
add_library(bt_lib STATIC
        connection_manager/connection_manager.cpp
        connection_manager/connection_manager.hpp
        services/ess/environmental_sensing_service.cpp services/ess/environmental_sensing_service.hpp connection_manager/connection_handler.cpp connection_manager/connection_handler.hpp adv_btn/advertise_button.cpp adv_btn/advertise_button.hpp shared/msg_passer.cpp shared/msg_passer.hpp)

target_include_directories(bt_lib   PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(bt_lib  PRIVATE
        zephyr_interface
        kernel
        utils_lib
        etl
        )