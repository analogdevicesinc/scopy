cmake_minimum_required(VERSION 3.9)
project(scopycore VERSION 0.1 LANGUAGES CXX)

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

FILE(GLOB SRC_LIST  *.cpp *.cc)
FILE(GLOB HEADER_LIST *.h i*.hpp)
FILE(GLOB UI_LIST *.ui)

add_subdirectory(test)

set(PROJECT_SOURCES
        ${SRC_LIST}
        ${HEADER_LIST}
        ${UI_LIST}
)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_library(${PROJECT_NAME} SHARED
    ${PROJECT_SOURCES}
)

generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}_export.h)

target_include_directories(${PROJECT_NAME} PUBLIC /home/adi/tmp/tool_launcher)
target_include_directories(${PROJECT_NAME} PUBLIC /home/adi/tmp/tool_launcher/gui)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PUBLIC ui)

target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Widgets scopygui iio)


