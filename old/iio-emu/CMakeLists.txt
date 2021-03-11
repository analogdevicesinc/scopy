# Copyright (c) 2021 Analog Devices Inc.
#
# This file is part of iio-emu
# (see http://www.github.com/analogdevicesinc/iio-emu).
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
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.1.3)
project(iio-emu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(IIO_EMU_VERSION_MAJOR 0)
set(IIO_EMU_VERSION_MINOR 1)
set(IIO_EMU_VERSION ${IIO_EMU_VERSION_MAJOR}.${IIO_EMU_VERSION_MINOR})

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(FindGit OPTIONAL)
if (GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --show-toplevel
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE IIO_EMU_GIT_REPO
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (${IIO_EMU_GIT_REPO} MATCHES ${CMAKE_CURRENT_SOURCE_DIR})
        execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE IIO_EMU_GIT_REPO
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()
endif()

find_package(Qt5 COMPONENTS Network Xml REQUIRED)
find_package(tinyiiod REQUIRED)

FILE(GLOB_RECURSE SRC_LIST
        iiod/*.cpp
        networking/*.cpp
        utils/*.cpp)
FILE(GLOB FILE_RESOURCES resources.qrc)
qt5_add_resources(IIO_EMU_RESOURCES ${FILE_RESOURCES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

add_executable(${PROJECT_NAME}
        main.cpp
        ${SRC_LIST}
        ${IIO_EMU_RESOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${IIO_EMU_VERSION}
        SOVERSION ${IIO_EMU_VERSION_MAJOR}
        )

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Network
        Qt5::Xml
        tinyiiod::tinyiiod)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
