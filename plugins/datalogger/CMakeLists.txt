cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE datalogger)

message(STATUS "building plugin: " ${SCOPY_MODULE})

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

FILE(GLOB SRC_LIST
	src/*.cpp
        src/*.hpp
        src/*.h
        src/gui/*.cpp
        src/gui/*.hpp
        src/gui/*.h
)

FILE(GLOB HEADER_LIST include/${SCOPY_MODULE}/*.h include/${SCOPY_MODULE}/*.hpp)
FILE(GLOB UI_LIST ui/*.ui)

if(ENABLE_TESTING)
    add_subdirectory(test)
endif()

set(PROJECT_SOURCES
	${SRC_LIST}
	${HEADER_LIST}
	${UI_LIST}
)

find_package(libm2k REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
if(NOT "${SCOPY_PLUGIN_INSTALL_PATH}" STREQUAL "")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SCOPY_PLUGIN_INSTALL_PATH})
endif()

if(ANDROID)
	find_library(QWT_LIBRARIES REQUIRED NAMES qwt_${ANDROID_ABI})
else()
	find_library(QWT_LIBRARIES REQUIRED NAMES qwt)
endif()


qt_add_resources(PROJECT_RESOURCES resources/resources.qrc ../../resources/resources.qrc)

add_library(${PROJECT_NAME} SHARED
	${PROJECT_SOURCES}
	${PROJECT_RESOURCES}
)

generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE})

include_directories(${Qt5Concurrent_INCLUDE_DIRS} ${GLIBCONFIG_INCLUDE_DIRS} ${Qt5Concurrent_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Widgets libm2k::libm2k scopypluginbase scopyiioutil scopygui scopycore ${QWT_LIBRARIES} ${GLIB_LIBRARIES})

set(PLUGIN_NAME ${PROJECT_NAME} PARENT_SCOPE)

install(TARGETS ${PROJECT_NAME}
	RUNTIME
	DESTINATION ${SCOPY_PLUGIN_INSTALL_DIR})


