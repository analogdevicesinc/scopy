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

set(SCOPY_MODULE regmap)

message(STATUS "building plugin: " ${SCOPY_MODULE})

project(scopy-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

set(PLUGIN_DISPLAY_NAME "Register map")
set(PLUGIN_DESCRIPTION "Register map of connected device with detailed register table for available XML templates")

include(GenerateExportHeader)

# TODO: split stylesheet/resources and add here TODO: export header files correctly

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

set(SCOPY_QT_COMPONENTS Widgets Xml Test)

file(
	GLOB
	SRC_LIST
	src/*.cpp
	src/*.cc
	src/*.hpp
	src/register/*.cpp
	src/register/*.hpp
	src/register/bitfield/*.cpp
	src/register/bitfield/*.hpp
	src/readwrite/*.cpp
	src/readwrite/*.hpp
	src/recyclerview/*.cpp
	src/recyclerview/*.hpp
)
file(GLOB HEADER_LIST include/${SCOPY_MODULE}/*.h include/${SCOPY_MODULE}/*.hpp)
file(GLOB UI_LIST ui/*.ui)

qt_add_resources(PROJECT_RESOURCES resources/resources.qrc)

if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST} ${UI_LIST})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${SCOPY_QT_COMPONENTS} REQUIRED)

find_package(libm2k REQUIRED)

set(REGMAP_XML_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/resources/regmap-xml)

file(GLOB XMLS ${CMAKE_CURRENT_SOURCE_DIR}/resources/regmap-xml/*.xml)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/regmap-xml/regmap-config.json DESTINATION ${REGMAP_XML_BUILD_PATH})
foreach(_xml ${XMLS})
	file(COPY ${_xml} DESTINATION ${REGMAP_XML_BUILD_PATH})
endforeach()

message(include/${SCOPY_MODULE}/)
configure_file(
	include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h.cmakein
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h @ONLY
)
set(SRC_LIST ${SRC_LIST} ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${PROJECT_RESOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

include(ScopyStyle)
generate_style(
	"--plugin" ${CMAKE_CURRENT_SOURCE_DIR}/style ${CMAKE_CURRENT_SOURCE_DIR}/include/regmap ${CMAKE_BINARY_DIR}
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

foreach(comp ${SCOPY_QT_COMPONENTS})
	set(SCOPY_QT_LIBRARIES ${SCOPY_QT_LIBRARIES} Qt${QT_VERSION_MAJOR}::${comp})
endforeach()

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC ${SCOPY_QT_LIBRARIES}
	       scopy-pluginbase
	       scopy-iioutil
	       scopy-gui
	       scopy-core
	       scopy-pkg-manager
	       Qt${QT_VERSION_MAJOR}::Xml
	       libm2k::libm2k
	       Qt5::Test
)

set(REGNMAP_TARGET_NAME ${PROJECT_NAME} PARENT_SCOPE)
