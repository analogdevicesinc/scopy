cmake_minimum_required(VERSION 3.9)
project(scopyplugins VERSION 0.1 LANGUAGES CXX)

set(SCOPY_PLUGIN_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/plugins/")
message(CHECK_START "plugins to " ${SCOPY_PLUGIN_INSTALL_PATH})

add_subdirectory(testplugin)
list(APPEND PLUGINS ${PLUGIN_NAME})
add_subdirectory(testplugin2)
list(APPEND PLUGINS ${PLUGIN_NAME})
add_subdirectory(swiot)
list(APPEND PLUGINS ${PLUGIN_NAME})
