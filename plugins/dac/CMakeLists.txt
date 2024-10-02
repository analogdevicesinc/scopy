cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE dac)

message(STATUS "building plugin: " ${SCOPY_MODULE})

project(scopy-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

set(PLUGIN_DISPLAY_NAME "DAC")
set(PLUGIN_DESCRIPTION "Digital to Analog Converter")

include(GenerateExportHeader)

# TODO: split stylesheet/resources and add here TODO: export header files correctly

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

file(GLOB SRC_LIST src/*.cpp src/*.cc)
file(GLOB HEADER_LIST include/${SCOPY_MODULE}/*.h include/${SCOPY_MODULE}/*.hpp)
file(GLOB UI_LIST ui/*.ui)

set(ENABLE_TESTING ON)
if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST} ${UI_LIST})
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS REQUIRED Widgets Core)

set(DAC_CSV_SYSTEM_PATH ${SCOPY_PLUGIN_INSTALL_PATH}/csv)
set(DAC_CSV_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/res/csv)

install(DIRECTORY res/csv DESTINATION ${SCOPY_PLUGIN_INSTALL_PATH})

file(GLOB CSVS ${CMAKE_CURRENT_SOURCE_DIR}/res/csv/*)
foreach(_csv ${CSVS})
	file(COPY ${_csv} DESTINATION ${DAC_CSV_BUILD_PATH})
endforeach()

qt_add_resources(PROJECT_RESOURCES res/resources.qrc)
add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${PROJECT_RESOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

configure_file(
	include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h.cmakein
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h @ONLY
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE})

target_include_directories(${PROJECT_NAME} PUBLIC scopy-pluginbase scopy-gui)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC Qt::Widgets
	       Qt::Core
	       scopy-pluginbase
	       scopy-gui
	       scopy-iioutil
	       scopy-iio-widgets
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(INSTALLER_DESCRIPTION "${PLUGIN_DISPLAY_NAME} ${PLUGIN_DESCRIPTION}")
	configureinstallersettings(${SCOPY_MODULE} ${INSTALLER_DESCRIPTION} FALSE)
endif()

set(DAC_TARGET_NAME ${PROJECT_NAME} PARENT_SCOPE)
