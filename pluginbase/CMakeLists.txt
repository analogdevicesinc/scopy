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

set(SCOPY_MODULE pluginbase)
project(scopy-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

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

file(GLOB SRC_LIST src/*.cpp src/*.cc)
file(
	GLOB
	HEADER_LIST
	src/*.hpp
	src/*.h
	include/${SCOPY_MODULE}/*.h
	include/${SCOPY_MODULE}/*.hpp
)
file(GLOB UI_LIST ui/*.ui)

if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

set(SCOPY_QT_COMPONENTS Core Widgets Concurrent)

option(ENABLE_SCOPYJS "Enable plugin scripting via JSEngine - requires QML" ON)
if(ENABLE_SCOPYJS)
	set(SCOPY_QT_COMPONENTS ${SCOPY_QT_COMPONENTS} Qml)
	message(STATUS "PLUGINBASE: scopy-js enabled")
endif()

configure_file(
	include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h.cmakein
	${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h @ONLY
)
set(SRC_LIST ${SRC_LIST} ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/scopy-${SCOPY_MODULE}_config.h)

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST} ${UI_LIST})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${SCOPY_QT_COMPONENTS} REQUIRED)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(
	${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}
				scopy-common scopy-iioutil
)

foreach(comp ${SCOPY_QT_COMPONENTS})
	set(SCOPY_QT_LIBRARIES ${SCOPY_QT_LIBRARIES} Qt${QT_VERSION_MAJOR}::${comp})
endforeach()

target_link_libraries(${PROJECT_NAME} PUBLIC ${SCOPY_QT_LIBRARIES} ${IIO_LIBRARIES} scopy-common scopy-iioutil)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SCOPY_DLL_INSTALL_PATH} COMPONENT ${SCOPY_PDK}
	RUNTIME DESTINATION ${SCOPY_DLL_INSTALL_PATH}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR} COMPONENT ${SCOPY_PDK})
