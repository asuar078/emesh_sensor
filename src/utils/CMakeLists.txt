
add_library(utils_lib
        STATIC
        logger.cpp
        logger.hpp
        )

target_include_directories(utils_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>

        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )


target_link_libraries(utils_lib
        PRIVATE
        zephyr_interface
        kernel
        )