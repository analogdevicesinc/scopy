cmake_minimum_required(VERSION 3.9.6)

project(gui LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)

# As the include/src files are split this does not work we will need to manually
# wrap cpp files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Test Qml REQUIRED)

add_library(
    gui SHARED
    # source files
    src/two_buttons_widget.cpp
    src/run_single_widget.cpp
    src/dynamic_widget.cpp
    src/subsection_separator.cpp
    src/settings_pair_widget.cpp
    src/custom_push_button.cpp
    src/spinbox_a.cpp
    src/api_object_manager.cpp
    src/completion_circle.cpp
    src/api_object.cpp
    src/plot_utils.cpp
    # ui files
    ui/two_buttons_widget.ui
    ui/run_single_widget.ui
    ui/subsection_separator.ui
    ui/settings_pair_widget.ui
    ui/spinbox_a.ui
    # resources
    resources/resources.qrc
)

# MOC hack in order to have the header files in include/scopy/gui
file(GLOB_RECURSE HEADERS_TO_MOC
     ${CMAKE_CURRENT_SOURCE_DIR}/include/scopy/gui/*.hpp
)
qt5_wrap_cpp(
    gui_moc
    ${HEADERS_TO_MOC}
    TARGET
    gui
    OPTIONS
    --no-notes
)
target_sources(gui PRIVATE ${gui_moc})
# end MOC hack

set_target_properties(
    gui PROPERTIES AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

target_include_directories(
    gui PUBLIC $<INSTALL_INTERFACE:include>
               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Qml
                project_options project_warnings
)

add_library(scopy::gui ALIAS gui)

add_subdirectory(tests)
