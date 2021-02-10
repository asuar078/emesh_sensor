
#add_library(utils_lib
#        STATIC
##        logger.cpp
##        logger.hpp
#
#        coap_client_utils.c
#        coap_client_utils.h
#        coap_server_client_interface.h
#
#        )
#
#target_include_directories(utils_lib
#        PUBLIC
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
#
#        INTERFACE
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
#        )
#
#
#target_link_libraries(utils_lib
#        PRIVATE
#        zephyr_interface
#        kernel
#        )

zephyr_library()
#zephyr_include_directories(.)

zephyr_sources(
        ../coap_client_utils.c
        )