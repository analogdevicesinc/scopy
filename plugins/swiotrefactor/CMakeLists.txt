cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE swiotrefactor)

message(STATUS "building plugin: " ${SCOPY_MODULE})

project(scopy-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

include(GenerateExportHeader)

# TODO: split stylesheet/resources and add here TODO: export header files correctly

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

message(CMAKE_AUTOUIC_SEARCH_PATHS="${CMAKE_AUTOUIC_SEARCH_PATHS}")

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
	src/*.cpp
	src/*.cc
	model/*
)

file(
	GLOB
	HEADER_LIST
	include/${SCOPY_MODULE}/*.h
	include/${SCOPY_MODULE}/*.hpp
	include/${SCOPY_MODULE}/max14906/*.h
	include/${SCOPY_MODULE}/max14906/*.hpp
)
file(GLOB UI_LIST ui/*.ui)

if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

message(UI_LIST= "${UI_LIST}")

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST} ${UI_LIST})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${SCOPY_QT_COMPONENTS} REQUIRED)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SCOPY_PLUGIN_BUILD_PATH})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Scopy.app/Contents/MacOS/plugins/plugins")
else()
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SCOPY_PLUGIN_BUILD_PATH})
endif()

if(ANDROID)
	find_library(QWT_LIBRARIES REQUIRED NAMES qwt_${ANDROID_ABI})
else()
	find_library(QWT_LIBRARIES REQUIRED NAMES qwt)
endif()

qt_add_resources(PROJECT_RESOURCES res/resources.qrc ../../resources/resources.qrc)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${PROJECT_RESOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE})

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
	       ${QWT_LIBRARIES}
)

set(PLUGIN_NAME ${PROJECT_NAME} PARENT_SCOPE)
