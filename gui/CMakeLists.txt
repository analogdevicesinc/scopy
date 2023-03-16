cmake_minimum_required(VERSION 3.9)
set(SCOPY_MODULE gui)
project(scopy${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

include(GenerateExportHeader)

# TODO: split stylesheet/resources and add here
# TODO: export header files correctly

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

FILE(GLOB SRC_LIST  src/*.cpp src/*.cc src/binding/*.cpp src/prop/*.cpp)
FILE(GLOB HEADER_LIST include/${SCOPY_MODULE}/*.h include/${SCOPY_MODULE}/*.hpp include/${SCOPY_MODULE}/binding/*.hpp include/${SCOPY_MODULE}/prop/*.hpp)
FILE(GLOB UI_LIST ui/*.ui)

find_package(Gnuradio "3.10" REQUIRED COMPONENTS runtime analog blocks fft filter volk pmt iio)
if(${Gnuradio_FOUND})
    find_package(gnuradio-scopy REQUIRED PATH_SUFFIXES scopy)
    set(SCOPYGUI_HAVE_GNURADIO ON)
endif()

find_package(PkgConfig)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
#pkg_check_modules(GLIB REQUIRED glib-2.0)
#pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
#pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)
pkg_check_modules(LIBSIGROK_DECODE REQUIRED libsigrokdecode)

pkg_get_variable(LIBSIGROK_DECODERS_DIR libsigrokdecode decodersdir)

if(ANDROID)
	find_library(QWT_LIBRARIES REQUIRED NAMES qwt_${ANDROID_ABI})
else()
	find_library(QWT_LIBRARIES REQUIRED NAMES qwt)
endif()

if(ENABLE_TESTING)
    add_subdirectory(test)
endif()

configure_file(include/${SCOPY_MODULE}/scopy${SCOPY_MODULE}_config.h.cmakein
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy${SCOPY_MODULE}_config.h @ONLY)

set(PROJECT_SOURCES
        ${SRC_LIST}
        ${HEADER_LIST}
        ${UI_LIST}
)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

message(STATUS "Building scopy-gui")
add_library(${PROJECT_NAME} SHARED
    ${PROJECT_SOURCES}
)

generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}include/${SCOPY_MODULE}/
    ${CMAKE_CURRENT_SOURCE_DIR}include/${SCOPY_MODULE}/binding
    ${CMAKE_CURRENT_SOURCE_DIR}include/${SCOPY_MODULE}/prop
    ${LIBSIGROK_DECODE_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE} ${QWT_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    ${QWT_LIBRARIES}
    gnuradio::gnuradio-runtime
    gnuradio::gnuradio-analog
    gnuradio::gnuradio-blocks
    gnuradio::gnuradio-fft
    gnuradio::gnuradio-filter
    gnuradio::gnuradio-pmt
    gnuradio::gnuradio-iio
    gnuradio::gnuradio-scopy
    ${LIBSIGROK_DECODE_LIBRARIES}
)
