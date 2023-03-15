cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE m2kplugin)

message(STATUS "building plugin: " ${SCOPY_MODULE})

project(scopy${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

include(GenerateExportHeader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

FILE(GLOB SRC_LIST  src/*.cpp src/*.cc)
FILE(GLOB OLD_SCOPY_FILES src/old/*.cpp src/old/*.hpp src/old/*.cc src/old/*.hh src/old/*.c src/old/*.h)

FILE(GLOB HEADER_LIST include/${SCOPY_MODULE}/*.h include/${SCOPY_MODULE}/*.hpp)
FILE(GLOB UI_LIST ui/*.ui)

set(ENABLE_TESTING ON)
if(ENABLE_TESTING)
    add_subdirectory(test)
endif()

set(PROJECT_SOURCES
        ${SRC_LIST}
	${OLD_SCOPY_FILES}
        ${HEADER_LIST}
        ${UI_LIST}
)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH};/usr/local/lib/cmake)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS REQUIRED Widgets Core )
find_package(Boost REQUIRED COMPONENTS system filesystem thread chrono)
find_package(libm2k REQUIRED)

find_package(Gnuradio "3.10" REQUIRED COMPONENTS runtime analog blocks fft filter volk pmt iio)
find_package(gnuradio-scopy REQUIRED PATH_SUFFIXES scopy)
find_package(gnuradio-m2k REQUIRED PATH_SUFFIXES m2k)

if(NOT "${SCOPY_PLUGIN_INSTALL_PATH}" STREQUAL "")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SCOPY_PLUGIN_INSTALL_PATH})
endif()

qt_add_resources(PROJECT_RESOURCES res/resources.qrc)

add_library(${PROJECT_NAME} SHARED
    ${PROJECT_SOURCES}
    ${PROJECT_RESOURCES}
)

generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE
	Qt::Widgets Qt::Core
	scopypluginbase
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE} ${CMAKE_CURRENT_SOURCE_DIR}/src/old
	)
target_include_directories(${PROJECT_NAME} PUBLIC scopypluginbase)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt::Widgets Qt::Core
    scopypluginbase
    scopyiioutil
    scopygui
    libm2k::libm2k
    gnuradio::gnuradio-runtime
    gnuradio::gnuradio-analog
    gnuradio::gnuradio-blocks
    gnuradio::gnuradio-fft
    gnuradio::gnuradio-filter
    gnuradio::gnuradio-pmt
    gnuradio::gnuradio-iio
    gnuradio::gnuradio-scopy
    gnuradio::gnuradio-m2k
)

set(PLUGIN_NAME ${PROJECT_NAME} PARENT_SCOPE)

install(TARGETS ${PROJECT_NAME}
    RUNTIME
    DESTINATION ${SCOPY_PLUGIN_INSTALL_DIR})
