#
# Copyright (c) 2024 Analog Devices Inc.
#
# This file is part of Scopy
# (see https://www.github.com/analogdevicesinc/scopy).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE m2k)

message(STATUS "building plugin: " ${SCOPY_MODULE})

project(scopy-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

set(PLUGIN_DISPLAY_NAME "ADALM2000")
set(PLUGIN_DESCRIPTION "Plugin for ADALM2000 (M2K)")

include(GenerateExportHeader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui ${CMAKE_CURRENT_SOURCE_DIR}/ui/patterns)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

set(SCOPY_QT_COMPONENTS Core Widgets UiTools)

file(GLOB SRC_LIST src/*.cpp src/*.cc)
file(
	GLOB
	OLD_SCOPY_FILES
	src/old/*.cpp
	src/old/logicanalyzer/*.cpp
	src/old/logicanalyzer/*.hpp
	src/old/patterngenerator/*.cpp
	src/old/patterngenerator/*.hpp
	src/old/patterngenerator/patterns/*.cpp
	src/old/patterngenerator/patterns/*.hpp
	src/old/*.hpp
	src/old/*.cc
	src/old/*.hh
	src/old/*.c
	src/old/*.h
)

file(GLOB HEADER_LIST include/${SCOPY_MODULE}/*.h include/${SCOPY_MODULE}/*.hpp)
file(GLOB UI_LIST ui/*.ui ui/patterns/*.ui)

set(ENABLE_TESTING ON)
if(ENABLE_TESTING)
	add_subdirectory(test)
endif()
add_subdirectory(m2k-gui)

set(PROJECT_SOURCES ${SRC_LIST} ${OLD_SCOPY_FILES} ${HEADER_LIST} ${UI_LIST})

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH};/usr/local/lib/cmake)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${SCOPY_QT_COMPONENTS} REQUIRED)
find_package(libm2k REQUIRED)

find_package(Boost COMPONENTS system filesystem thread chrono REQUIRED) # workaround for Gnuradio not finding
# Boost::chrono
find_package(
	Gnuradio "3.10" REQUIRED
	COMPONENTS runtime
		   analog
		   blocks
		   fft
		   filter
		   volk
		   pmt
		   iio
)
find_package(gnuradio-scopy REQUIRED PATH_SUFFIXES scopy)
find_package(gnuradio-m2k REQUIRED PATH_SUFFIXES m2k)

qt_add_resources(PROJECT_RESOURCES res/resources.qrc)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${PROJECT_RESOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

configure_file(
	include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h.cmakein
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h @ONLY
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE Qt::Widgets
		Qt::Core
		scopy-m2k-gui
		scopy-pluginbase
		scopy-common
		${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}
		${CMAKE_CURRENT_SOURCE_DIR}/src/old
)
target_include_directories(${PROJECT_NAME} PUBLIC scopy-pluginbase ${QWT_INCLUDE_DIRS})

foreach(comp ${SCOPY_QT_COMPONENTS})
	set(SCOPY_QT_LIBRARIES ${SCOPY_QT_LIBRARIES} Qt${QT_VERSION_MAJOR}::${comp})
endforeach()

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC ${SCOPY_QT_LIBRARIES}
	       ${QWT_LIBRARIES}
	       scopy-common
	       scopy-pluginbase
	       scopy-iioutil
	       scopy-gui
	       scopy-m2k-gui
	       scopy-gr-gui
	       scopy-sigrok-gui
	       libm2k::libm2k
	       gnuradio::gnuradio-runtime
	       gnuradio::gnuradio-analog
	       gnuradio::gnuradio-blocks
	       gnuradio::gnuradio-fft
	       gnuradio::gnuradio-filter
	       gnuradio::gnuradio-pmt
	       gnuradio::gnuradio-iio
	       gnuradio::gnuradio-scopy
	       gnuradio::gnuradio-m2k
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(INSTALLER_DESCRIPTION "${PLUGIN_DISPLAY_NAME} ${PLUGIN_DESCRIPTION}")
	configureinstallersettings(${SCOPY_MODULE} ${INSTALLER_DESCRIPTION} "disablenouninstallwarning")
endif()

set(M2K_TARGET_NAME ${PROJECT_NAME} PARENT_SCOPE)
