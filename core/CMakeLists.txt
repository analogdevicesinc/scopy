cmake_minimum_required(VERSION 3.9.6)

project(core LANGUAGES CXX)

# As the include/src files are split this does not work we will need to manually
# wrap cpp files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Qml REQUIRED)

add_library(
    core SHARED
    # source files
    src/api_object.cpp src/api_object_manager.cpp
)

# MOC hack in order to have the header files in include/scopy/gui
file(GLOB_RECURSE HEADERS_TO_MOC
     ${CMAKE_CURRENT_SOURCE_DIR}/include/scopy/core/*.hpp
)
qt5_wrap_cpp(
    gui_moc
    ${HEADERS_TO_MOC}
    TARGET
    gui
    OPTIONS
    --no-notes
)
target_sources(core PRIVATE ${gui_moc})
# end MOC hack

target_include_directories(
    core PUBLIC $<INSTALL_INTERFACE:include>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    core PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Qml
                 project_options project_warnings
)

add_library(scopy::core ALIAS core)
