# Copyright 2016 Analog Devices, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3, or (at your option)
#  any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with GNU Radio; see the file COPYING.  If not, write to
#  the Free Software Foundation, Inc., 51 Franklin Street,
#  Boston, MA 02110-1301, USA.

project(scopy)
cmake_minimum_required(VERSION 3.1.3)

if (MSVC)
	# http://www.suodenjoki.dk/us/archive/2010/min-max.htm
	add_definitions(-DNOMINMAX)

	# http://www.qtcentre.org/threads/32028-unresolved-external-symbol-quot-public-static-struct-QMetaObject-const-QwtPlot-staticMe
	add_definitions(-DQWT_DLL)
endif (MSVC)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)  - Can't figure out how to configure it to look under ui/
#                         for the .ui file. Use qt5_wrap_ui instead.

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS None Debug Release RelWithDebInfo MinSizeRel)
endif()

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	add_definitions(-DQT_NO_DEBUG_OUTPUT=1)
endif()

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_constexpr OUT_CONSTEXPR)
if (NOT "${OUT_CONSTEXPR}" STREQUAL "-1")
	add_definitions(-DHAS_CONSTEXPR=1)
endif()

find_package(Qt5Widgets REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qwt REQUIRED)
find_package(Qt5Qml REQUIRED)

set(GR_REQUIRED_COMPONENTS RUNTIME ANALOG BLOCKS FFT FILTER IIO VOLK PMT)
find_package(Gnuradio PATHS "c:/Program Files/gnuradio/lib/cmake/gnuradio" REQUIRED)

GR_MODULE(IIO gnuradio-iio gnuradio/iio/api.h gnuradio-iio)

add_definitions(-DBOOST_ALL_DYN_LINK)
find_package(Boost "1.58" COMPONENTS system filesystem thread REQUIRED)

set(VC_PATH "$ENV{VS120COMNTOOLS}/../../VC")
find_library(IIO_LIBRARIES NAMES iio libiio PATHS ${VC_PATH}/lib ${VC_PATH}/lib/amd64)
find_path(IIO_INCLUDE_DIRS iio.h PATHS ${VC_PATH}/include)

include(GNUInstallDirs)

find_library(GLIB_LIBRARIES glib-2.0)
find_library(GLIBMM_LIBRARIES NAMES glibmm-2.4 glibmm-2.44)
find_library(SIGCPP_LIBRARIES sigc-2.4)
find_library(LIBSIGROK_LIBRARIES sigrok)
find_library(LIBSIGROKCXX_LIBRARIES sigrokcxx)
find_library(LIBSIGROK_DECODE_LIBRARIES sigrokdecode)

if (WIN32)
	find_library(PYTHON_LIBRARIES python36)
	find_path(GLIB_INCLUDE_DIRS glib.h PATH_SUFFIXES glib-2.0)
	find_path(GLIBCONFIG_INCLUDE_DIRS glibconfig.h PATH_SUFFIXES ../lib/glib-2.0/include)
	find_path(GLIBMM_INCLUDE_DIRS glibmm.h PATH_SUFFIXES glibmm-2.44)
	find_path(GLIBMMCONFIG_INCLUDE_DIRS glibmmconfig.h PATH_SUFFIXES ../lib/glibmm-2.44/include)
	find_path(SIGCPP_INCLUDE_DIRS sigc++/sigc++.h PATH_SUFFIXES sigc++-2.4)
	find_path(SIGCPPCONFIG_INCLUDE_DIRS sigc++config.h PATH_SUFFIXES ../lib/sigc++-2.4/include)
	find_path(LIBSIGROK_INCLUDE_DIRS libsigrok/libsigrok.h)
	find_path(LIBSIGROKCXX_INCLUDE_DIRS libsigrokcxx/libsigrokcxx.hpp)
	find_path(LIBSIGROK_DECODE_INCLUDE_DIRS libsigrokdecode/libsigrokdecode.h)
else (WIN32)
	find_package(PkgConfig)
	pkg_check_modules(PYTHON REQUIRED python3)
	pkg_check_modules(GLIB REQUIRED glib-2.0)
	pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
	pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)
	pkg_check_modules(LIBSIGROK REQUIRED libsigrok)
	pkg_check_modules(LIBSIGROKCXX REQUIRED libsigrokcxx)
	pkg_check_modules(LIBSIGROK_DECODE REQUIRED libsigrokdecode)
endif (WIN32)

include_directories(
	${GNURADIO_ALL_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5WebKitWidgets_INCLUDE_DIRS}
	${Qt5Concurrent_INCLUDE_DIRS}
	${Qt5Qml_INCLUDE_DIRS}
	${QWT_INCLUDE_DIRS}
	${IIO_INCLUDE_DIRS}
	${LIBSIGROK_DECODE_INCLUDE_DIRS}
	${LIBSIGROKCXX_INCLUDE_DIRS}
	${LIBSIGROK_INCLUDE_DIRS}
	${GLIB_INCLUDE_DIRS}
	${GLIBCONFIG_INCLUDE_DIRS}
	${GLIBMM_INCLUDE_DIRS}
	${GLIBMMCONFIG_INCLUDE_DIRS}
	${SIGCPP_INCLUDE_DIRS}
	${SIGCPPCONFIG_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src
)

link_directories(
	${Boost_LIBRARY_DIRS}
	${GNURADIO_RUNTIME_LIBRARY_DIRS}
)

FILE(GLOB SRC_LIST  src/*.cpp src/*.cc
	src/pulseview/pv/*.cpp
	src/pulseview/pv/binding/*.cpp
	src/pulseview/pv/data/*.cpp
	src/pulseview/pv/data/decode/*.cpp
	src/pulseview/pv/devices/*.cpp
	src/pulseview/pv/dialogs/*.cpp
	src/pulseview/pv/popups/*.cpp
	src/pulseview/pv/prop/*.cpp
	src/pulseview/pv/toolbars/*.cpp
	src/pulseview/pv/view/*.cpp
	src/pulseview/pv/widgets/*.cpp
)

FILE(GLOB M2KSCOPE_UIS ui/*.ui src/pulseview/pv/dialogs/*.ui)
qt5_wrap_ui (m2kscope_FORMS_HEADERS ${M2KSCOPE_UIS})

FILE(GLOB M2KSCOPE_RESOURCES resources/*.qrc)
qt5_add_resources(m2kscope_RESOURCES ${M2KSCOPE_RESOURCES})

set(RESOURCES resources/icon.rc)

add_definitions(-DQT_NO_KEYWORDS)

add_executable(${PROJECT_NAME} WIN32
		${SRC_LIST}
		${RESOURCES}
		${m2kscope_RESOURCES}
		${m2kscope_FORMS_HEADERS}
)

link_directories(
	${Boost_LIBRARY_DIRS}
	${GNURADIO_RUNTIME_LIBRARY_DIRS}
)

target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
		${Qt5Widgets_LIBRARIES}
		${Qt5WebKitWidgets_LIBRARIES}
		${Qt5Concurrent_LIBRARIES}
		${Qt5Qml_LIBRARIES}
		${GNURADIO_ALL_LIBRARIES}
		${Boost_LIBRARIES}
		${QWT_LIBRARIES}
		${IIO_LIBRARIES}
		${PYTHON_LIBRARIES}
		${LIBSIGROK_LIBRARIES}
		${LIBSIGROKCXX_LIBRARIES}
		${LIBSIGROK_DECODE_LIBRARIES}
		${GLIBMM_LIBRARIES}
		${SIGCPP_LIBRARIES}
		${GLIB_LIBRARIES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
		CXX_STANDARD 11
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
)
