cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE swiot)

message(STATUS "building plugin: " ${SCOPY_MODULE})

project(scopy-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

set(PLUGIN_DISPLAY_NAME "AD-SWIOT1L-SL")
set(PLUGIN_DESCRIPTION "Plugin for AD-SWIOT1L-SL")

include(GenerateExportHeader)

# TODO: split stylesheet/resources and add here TODO: export header files correctly

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

set(SCOPY_QT_COMPONENTS Widgets Concurrent Svg Xml)

file(
	GLOB
	SRC_LIST
	# some of these will eventually be moved to the core/gui directories
	src/max14906/*.cpp
	src/max14906/*.cc
	src/faults/*.cpp
	src/faults/*.cc
	src/ad74413r/*.cpp
	src/ad74413r/*.cc
	src/*.cpp
	src/*.cc
	src/config/*.cpp
	src/config/*.cc
	model/*
)

file(
	GLOB
	HEADER_LIST
	include/${SCOPY_MODULE}/*.h
	include/${SCOPY_MODULE}/*.hpp
	include/${SCOPY_MODULE}/max14906/*.h
	include/${SCOPY_MODULE}/max14906/*.hpp
	include/${SCOPY_MODULE}/faults/*.h
	include/${SCOPY_MODULE}/faults/*.hpp
	include/${SCOPY_MODULE}/ad74413r/*.h
	include/${SCOPY_MODULE}/ad74413r/*.hpp
	include/${SCOPY_MODULE}/config/*.h
	include/${SCOPY_MODULE}/config/*.hpp
)

if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${SCOPY_QT_COMPONENTS} REQUIRED)

qt_add_resources(PROJECT_RESOURCES res/resources.qrc ../../resources/resources.qrc)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${PROJECT_RESOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

configure_file(
	include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h.cmakein
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h @ONLY
)

set(INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE} ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/ad74413r
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/faults
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/max14906
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRECTORIES})

include_directories(${Qt${QT_VERSION_MAJOR}Concurrent_INCLUDE_DIRS})

foreach(comp ${SCOPY_QT_COMPONENTS})
	set(SCOPY_QT_LIBRARIES ${SCOPY_QT_LIBRARIES} Qt${QT_VERSION_MAJOR}::${comp})
endforeach()

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC ${SCOPY_QT_LIBRARIES}
	       scopy-pluginbase
	       scopy-iioutil
	       scopy-gui
	       scopy-core
	       scopy-iio-widgets
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(INSTALLER_DESCRIPTION "${PLUGIN_DISPLAY_NAME} ${PLUGIN_DESCRIPTION}")
	configureinstallersettings(${SCOPY_MODULE} ${INSTALLER_DESCRIPTION} TRUE)
endif()

set(SWIOT_TARGET_NAME ${PROJECT_NAME} PARENT_SCOPE)
