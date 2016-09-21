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

set(GR_REQUIRED_COMPONENTS RUNTIME ANALOG BLOCKS FFT FILTER IIO VOLK PMT)
find_package(Gnuradio PATHS "c:/Program Files/gnuradio/lib/cmake/gnuradio" REQUIRED)

GR_MODULE(IIO gnuradio-iio gnuradio/iio/api.h gnuradio-iio)

add_definitions(-DBOOST_ALL_DYN_LINK)
find_package(Boost "1.58" COMPONENTS system REQUIRED)

set(VC_PATH "$ENV{VS120COMNTOOLS}/../../VC")
find_library(IIO_LIBRARIES NAMES iio libiio PATHS ${VC_PATH}/lib ${VC_PATH}/lib/amd64)
find_path(IIO_INCLUDE_DIRS iio.h PATHS ${VC_PATH}/include)

include_directories(
	${GNURADIO_ALL_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5WebKitWidgets_INCLUDE_DIRS}
	${Qt5Concurrent_INCLUDE_DIRS}
	${QWT_INCLUDE_DIRS}
	${IIO_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src
)

link_directories(
	${Boost_LIBRARY_DIRS}
	${GNURADIO_RUNTIME_LIBRARY_DIRS}
)

FILE(GLOB SRC_LIST src/*.cpp src/*.cc)

FILE(GLOB M2KSCOPE_UIS ui/*.ui)
qt5_wrap_ui (m2kscope_FORMS_HEADERS ${M2KSCOPE_UIS})

FILE(GLOB M2KSCOPE_RESOURCES resources/*.qrc)
qt5_add_resources(m2kscope_RESOURCES ${M2KSCOPE_RESOURCES})

set(RESOURCES resources/icon.rc)

add_executable(${PROJECT_NAME} WIN32
		${SRC_LIST}
		${RESOURCES}
		${m2kscope_RESOURCES}
		${m2kscope_FORMS_HEADERS}
)

target_link_libraries(${PROJECT_NAME} LINK_PRIVATE
		${Qt5Widgets_LIBRARIES}
		${Qt5WebKitWidgets_LIBRARIES}
		${Qt5Concurrent_LIBRARIES}
		${GNURADIO_ALL_LIBRARIES}
		${Boost_LIBRARIES}
		${QWT_LIBRARIES}
		${IIO_LIBRARIES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
		CXX_STANDARD 11
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
)
