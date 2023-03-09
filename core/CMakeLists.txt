cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE core)
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

set(SCOPY_QT_COMPONENTS Core Widgets Concurrent)

FILE(GLOB SRC_LIST  src/*.cpp src/*.cc)
FILE(GLOB HEADER_LIST include/${SCOPY_MODULE}/*.h include/${SCOPY_MODULE}/*.hpp)
FILE(GLOB UI_LIST ui/*.ui)

option(ENABLE_SCOPYJS "Enable plugin scripting via JSEngine - requires QML" ON)

if(ENABLE_TESTING)
    add_subdirectory(test)
endif()

if(ENABLE_SCOPYJS)
    set(SCOPY_QT_COMPONENTS ${SCOPY_QT_COMPONENTS} Qml)
    set(SCOPY_JS_INCLUDE_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopyjs)
    FILE(GLOB JS_SRC_LIST src/scopyjs/*.cpp src/scopyjs/*.h)    
    FILE(GLOB JS_HEADER_LIST include/${SCOPY_MODULE}/scopyjs/*.h include/scopyjs/${SCOPY_MODULE}/*.hpp)

    message(STATUS "CORE: scopy-js enabled")
    set(SRC_LIST ${SRC_LIST} ${JS_SRC_LIST})
    set(HEADER_LIST ${HEADER_LIST} ${JS_HEADER_LIST})
endif()

configure_file(include/core/scopy${SCOPY_MODULE}_config.h.cmakein
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy${SCOPY_MODULE}_config.h @ONLY)
set(SRC_LIST ${SRC_LIST} ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy${SCOPY_MODULE}_config.h)

set(PROJECT_SOURCES
        ${SRC_LIST}
        ${HEADER_LIST}
        ${UI_LIST}
)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${SCOPY_QT_COMPONENTS} REQUIRED)

add_library(${PROJECT_NAME} SHARED
    ${PROJECT_SOURCES}
)

generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE} ${SCOPY_JS_INCLUDE_LOCATION})
target_include_directories(${PROJECT_NAME} PUBLIC scopygui scopypluginbase scopyiioutil)

foreach(comp ${SCOPY_QT_COMPONENTS})
    set(SCOPY_QT_LIBRARIES ${SCOPY_QT_LIBRARIES} Qt${QT_VERSION_MAJOR}::${comp})
endforeach()

target_link_libraries(${PROJECT_NAME} PUBLIC ${SCOPY_QT_LIBRARIES} scopygui scopypluginbase scopyiioutil )








