#
# Copyright (c) 2026 Analog Devices Inc.
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

cmake_minimum_required(VERSION 3.16)

set(SCOPY_MODULE ad9361regmap)

message(STATUS "building plugin: " ${SCOPY_MODULE})

project(scopy-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

set(PLUGIN_DISPLAY_NAME "AD9361 Regmap Demo")
set(PLUGIN_DESCRIPTION "Configures the Register Map plugin to use file backed register access for ad9361-phy")

include(GenerateExportHeader)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

file(GLOB SRC_LIST src/*.cpp src/*.h)
file(GLOB HEADER_LIST include/${SCOPY_MODULE}/*.h)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS REQUIRED Widgets Core)

configure_file(
	include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h.cmakein
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h @ONLY
)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST} ${HEADER_LIST})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# scopy-regmap is built by the generic-plugins package, its public headers come from its INTERFACE include dir
target_link_libraries(
	${PROJECT_NAME}
	PUBLIC Qt::Widgets
	       Qt::Core
	       scopy-common
	       scopy-pluginbase
	       scopy-core
	       scopy-pkg-manager
	       component
	       scopy-regmap
)

# Find libscopy-regmap in the generic-plugins package at runtime
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/../../generic-plugins/plugins")
