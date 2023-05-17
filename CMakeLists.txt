cmake_minimum_required(VERSION 3.9)
enable_testing()

project(scopy VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if (Qt5Widgets_VERSION VERSION_LESS 5.15.2)
	message(FATAL_ERROR "Minimum supported Qt 5.15.2")
	return()
else()
	message(STATUS "Using Qt version: "${Qt5Widgets_VERSION})
endif()

#message(QtVersion: ${QT_VERSION_MAJOR}:${QT_VERSION_MINOR})

# Make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake/Modules)

FILE(GLOB SRC_LIST  *.cpp *.cc)
FILE(GLOB HEADER_LIST *.h *.hpp)
FILE(GLOB UI_LIST *.ui)

set(PROJECT_SOURCES
        ${SRC_LIST}
        ${HEADER_LIST}
        ${UI_LIST}
)

include(ScopyAbout)
configure_about(./resources/about)
FILE(GLOB SCOPY_RESOURCE_FILES gui/res/resources.qrc resources/aboutpage.qrc)
message(STATUS ${SCOPY_RESOURCE_FILES})
qt_add_resources(SCOPY_RESOURCES ${SCOPY_RESOURCE_FILES})

find_library(IIO_LIBRARIES NAMES iio libiio)
find_path(IIO_INCLUDE_DIRS iio.h)

option(ENABLE_TESTING "Enable unit tests" ON)
if(ENABLE_TESTING)
    message(STATUS "Unit tests enabled")
    add_subdirectory(tests)
endif()

add_subdirectory(gui)
add_subdirectory(iioutil)
add_subdirectory(pluginbase)
add_subdirectory(plugins)
add_subdirectory(core)

set(SCOPY_DEPENDENCIES ${IIO_LIBRARIES})



if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
	${PROJECT_SOURCES}
	${SCOPY_RESOURCES}
	${SCOPY_DEPENDENCIES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET tool_launcher APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
	add_library(${PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
            ${SCOPY_RESOURCES}
            ${SCOPY_DEPENDENCIES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
	add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
            ${SCOPY_RESOURCES}
            ${SCOPY_DEPENDENCIES}
        )
    endif()
endif()

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_constexpr OUT_CONSTEXPR)
add_definitions(-DQT_NO_KEYWORDS)

set(CMAKE_VERBOSE_MAKEFILE ON)
target_include_directories(${PROJECT_NAME} PUBLIC scopy-gui scopy-core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${SCOPY_DEPENDENCIES} scopy-core scopy-gui)

# Compiler options
target_compile_options(${PROJECT_NAME} PUBLIC -Wall)

#List of warnings to be treated as errors
target_compile_options(${PROJECT_NAME} PUBLIC
        -Werror=return-type
        -Werror=uninitialized
        -Werror=init-self
        -Werror=switch
)


set_target_properties(${PROJECT_NAME}  PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

configure_file(windows/scopy.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/windows/scopy.iss @ONLY)
configure_file(windows/scopy-32.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/windows/scopy-32.iss @ONLY)
configure_file(windows/scopy-64.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/windows/scopy-64.iss @ONLY)
configure_file(windows/qt.conf.cmakein ${CMAKE_CURRENT_BINARY_DIR}/qt.conf COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/scopy.desktop.cmakein ${CMAKE_CURRENT_BINARY_DIR}/scopy.desktop @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scopy.desktop DESTINATION share/applications)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gui/res/icon_small.svg DESTINATION share/icons/hicolor/scalable/apps RENAME scopy.svg)

install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION .)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
