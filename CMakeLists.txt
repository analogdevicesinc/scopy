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
message(QtVersion: ${QT_VERSION_MAJOR})

# Make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake/Modules)


add_subdirectory(iioutil)
add_subdirectory(pluginbase)
add_subdirectory(gui)
add_subdirectory(plugins)
add_subdirectory(core)

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
message(${SCOPY_RESOURCE_FILES})
qt_add_resources(SCOPY_RESOURCES ${SCOPY_RESOURCE_FILES})

find_library(IIO_LIBRARIES NAMES iio libiio)
find_path(IIO_INCLUDE_DIRS iio.h)


set(SCOPY_DEPENDENCIES ${IIO_LIBRARIES})

option(ENABLE_TESTING "Enable building unit tests" ON)
if(ENABLE_TESTING)
    message(STATUS "Unit tests enabled")
    add_subdirectory(tests)
endif()

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
if (NOT "${OUT_CONSTEXPR}" STREQUAL "-1")
	add_definitions(-DHAS_CONSTEXPR=1)
endif()
add_definitions(-DQT_NO_KEYWORDS)

set(CMAKE_VERBOSE_MAKEFILE ON)
target_include_directories(${PROJECT_NAME} PUBLIC scopygui scopycore)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${SCOPY_DEPENDENCIES} scopycore scopygui)

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

install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION .)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
