cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE datalogger)

message(STATUS "building plugin: " ${SCOPY_MODULE})

project(scopy-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

set(PLUGIN_DISPLAY_NAME "Data logger")
set(PLUGIN_DESCRIPTION "Used to monitor and log data")

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

file(
	GLOB
	SRC_LIST
	src/*.cpp
	src/*.hpp
	src/menus/*.cpp
	src/datamonitor/*.cpp
	src/datamonitor/readstrategy/*.cpp
)
file(
	GLOB
	HEADER_LIST
	include/${SCOPY_MODULE}/*.h
	include/${SCOPY_MODULE}/*.hpp
	include/${SCOPY_MODULE}/menus/*.h
	include/${SCOPY_MODULE}/menus/*.hpp
	include/${SCOPY_MODULE}/datamonitor/*.h
	include/${SCOPY_MODULE}/datamonitor/*.hpp
	include/${SCOPY_MODULE}/datamonitor/readstrategy/*.h
	include/${SCOPY_MODULE}/datamonitor/readstrategy/*.hpp
)
file(GLOB UI_LIST ui/*.ui)

set(ENABLE_TESTING ON)
if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST} ${UI_LIST})
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS REQUIRED Widgets Core)

qt_add_resources(PROJECT_RESOURCES resources/resources.qrc ../../resources/resources.qrc)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${PROJECT_RESOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

configure_file(
	include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h.cmakein
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h @ONLY
)

set(INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE} ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/menus
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/datamonitor
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/datamonitor/readstrategy
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRECTORIES})

target_include_directories(${PROJECT_NAME} PUBLIC scopy-pluginbase scopy-gui)
target_include_directories(${PROJECT_NAME} PRIVATE ${IIO_INCLUDE_DIRS} scopy-gui scopy-iio-widgets scopy-iioutil)

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

set(DATALOGGER_TARGET_NAME ${PROJECT_NAME} PARENT_SCOPE)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${SCOPY_PLUGIN_INSTALL_DIR})
