cmake_minimum_required(VERSION 3.9)
project(scopyplugins VERSION 0.1 LANGUAGES CXX)
set(SCOPY_PLUGIN_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/plugins/")
message(STATUS "plugins to " ${SCOPY_PLUGIN_INSTALL_PATH})

option(ENABLE_PLUGIN_TEST ON)
option(ENABLE_PLUGIN_M2K ON)
option(ENABLE_PLUGIN_SWIOT ON)

if(ENABLE_PLUGIN_TEST)
    add_subdirectory(testplugin)
    list(APPEND PLUGINS ${PLUGIN_NAME})
    add_subdirectory(testplugin2)
    list(APPEND PLUGINS ${PLUGIN_NAME})
    add_subdirectory(bareminimumplugin)
    list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

if(ENABLE_PLUGIN_SWIOT)
    add_subdirectory(swiot)
    list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

if(ENABLE_PLUGIN_M2K)
    add_subdirectory(m2k)
    list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

message(STATUS "Building plugins:" ${PLUGINS})

