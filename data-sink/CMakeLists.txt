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

set(SCOPY_MODULE data-sink)
project(scopy-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

include(GenerateExportHeader)

# TODO: split stylesheet/resources and add here TODO: export header files correctly

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

file(
	GLOB
	SRC_LIST
	src/*.cpp
)
file(
	GLOB
	HEADER_LIST
	include/${SCOPY_MODULE}/*.h
	include/${SCOPY_MODULE}/*.hpp
)

if(ENABLE_TESTING)
	add_subdirectory(test)
endif()

set(SCOPY_QT_COMPONENTS Core)

find_library(IIO_LIBRARIES NAMES iio libiio REQUIRED)
find_path(IIO_INCLUDE_DIRS iio.h REQUIRED)

set(PROJECT_SOURCES ${SRC_LIST} ${HEADER_LIST})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${SCOPY_QT_COMPONENTS} REQUIRED)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

generate_export_header(
	${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}/${PROJECT_NAME}_export.h
)

target_include_directories(
	${PROJECT_NAME} INTERFACE ${IIO_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include
				  ${CMAKE_CURRENT_SOURCE_DIR}/include/${SCOPY_MODULE}
)
target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
		${IIO_INCLUDE_DIRS}
)

target_include_directories(${PROJECT_NAME} PUBLIC scopy-common scopy-gui scopy-core scopy-iioutil)

foreach(comp ${SCOPY_QT_COMPONENTS})
	set(SCOPY_QT_LIBRARIES ${SCOPY_QT_LIBRARIES} Qt${QT_VERSION_MAJOR}::${comp})
endforeach()

target_link_libraries(${PROJECT_NAME} PUBLIC ${SCOPY_QT_LIBRARIES} ${IIO_LIBRARIES} scopy-gui scopy-iioutil)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SCOPY_DLL_INSTALL_PATH} COMPONENT ${SCOPY_PDK}
	RUNTIME DESTINATION ${SCOPY_DLL_INSTALL_PATH}
)
install(DIRECTORY include/ DESTINATION include/ COMPONENT ${SCOPY_PDK})
