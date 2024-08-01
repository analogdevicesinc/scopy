cmake_minimum_required(VERSION 3.9)

# Project name
set(TARGET_NAME "ScopyPluginRunner")

project(${"${TARGET_NAME}"} VERSION 0.0.1 DESCRIPTION "Project Description")

# Make sure CMake will take care of moc for us
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PDK_DEPS_PATH ${deps_path})
if(NOT DEFINED PDK_DEPS_PATH)
	message(FATAL_ERROR "PDK_DEPS_PATH is required!")
else()
	if(NOT EXISTS ${"${PDK_DEPS_PATH}"})
		message(FATAL_ERROR "The path=" \"${"${PDK_DEPS_PATH}"}\" " to the dependencies doesn't exist!")
	endif()
endif()
set(PDK_DEPS_INCLUDE ${"${PDK_DEPS_PATH}"}/usr/local/include)
if(NOT EXISTS ${"${PDK_DEPS_INCLUDE}"})
	message(FATAL_ERROR "The path=" \"${"${PDK_DEPS_INCLUDE}"}\" " to the headers doesn't exist!")
endif()

set(PDK_DEPS_LIB ${"${PDK_DEPS_PATH}"}/usr/local/lib)
if(NOT EXISTS ${"${PDK_DEPS_LIB}"})
	message(FATAL_ERROR "The path=" \"${"${PDK_DEPS_LIB}"}\" " to the libraries doesn't exist!")
endif()

set(PLUGIN_INSTALL_PATH ${"${CMAKE_CURRENT_BINARY_DIR}"}/plugin/${plugin_dir}/libscopy-${plugin_name}.so)
set(PATH_TO_INI ${preferences_path})

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${"${QT_VERSION_MAJOR}"} REQUIRED COMPONENTS Widgets Core)

file(GLOB SRC_LIST src/*.cpp)
file(GLOB HEADER_LIST include/*.h include/*.hpp)

configure_file(include/pdk-util_config.h.cmakein ${"${CMAKE_CURRENT_SOURCE_DIR}"}/include/pdk-util_config.h @ONLY)

set(PROJECT_SOURCES ${"${SRC_LIST}"} ${"${HEADER_LIST}"} ${"${CMAKE_CURRENT_SOURCE_DIR}"}/include/pdk-util_config.h)
find_path(IIO_INCLUDE_DIRS iio.h REQUIRED)
find_library(IIO_LIBRARIES NAMES iio libiio REQUIRED)

add_subdirectory(plugin/${plugin_dir})

qt_add_resources(PROJ_RES res/resources.qrc)

add_executable(${"${TARGET_NAME}"} ${"${PROJECT_SOURCES}"} ${"${PROJ_RES}"})

include(${"${CMAKE_CURRENT_SOURCE_DIR}"}/PdkSupport.cmake)

target_include_directories(${"${TARGET_NAME}"} PRIVATE ${"${Qt${QT_VERSION_MAJOR}_INCLUDE_DIRS}"})
target_include_directories(${"${TARGET_NAME}"} PRIVATE ${"${CMAKE_SOURCE_DIR}"}/include)
target_include_directories(${"${TARGET_NAME}"} INTERFACE ${"${IIO_INCLUDE_DIRS}"})

target_include_directories(${"${TARGET_NAME}"} PUBLIC ${"${PDK_DEPS_INCLUDE}"} ${"${IIO_INCLUDE_DIRS}"})

inlcude_dirs(${"${PDK_DEPS_INCLUDE}"})
# Add any extra libs to link also.
link_libs(${"${PDK_DEPS_LIB}"})
target_link_libraries(
	${"${TARGET_NAME}"} PRIVATE Qt${"${QT_VERSION_MAJOR}"}::Widgets Qt${"${QT_VERSION_MAJOR}"}::Core ${"${IIO_LIBRARIES}"}
)
