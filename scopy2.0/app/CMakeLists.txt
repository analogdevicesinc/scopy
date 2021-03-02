cmake_minimum_required(VERSION 3.9.6)

project(scopy LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

add_executable(scopy mainwindow.cpp main.cpp mainwindow.ui)

target_link_libraries(
    scopy PRIVATE Qt${QT_VERSION_MAJOR}::Widgets scopy::gui project_options
                  project_warnings
)
