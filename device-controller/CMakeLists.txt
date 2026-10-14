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
project(device-controller VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Test)
find_package(QCoro6 REQUIRED COMPONENTS Core Coro)

# libiio backend - chosen at BUILD time. Exactly one backend (iio-backend-v0 or iio-backend-v1) is built as a static
# library and linked into `component`, which calls its createIIOBackend() directly. Default libiio = 1 (also declared in
# the root CMakeLists.txt).
set(SCOPY_LIBIIO_VERSION "1" CACHE STRING "libiio API used by the device-controller backend (0 or 1)")
set_property(CACHE SCOPY_LIBIIO_VERSION PROPERTY STRINGS 0 1)
if(NOT SCOPY_LIBIIO_VERSION MATCHES "^[01]$")
	message(FATAL_ERROR "SCOPY_LIBIIO_VERSION must be 0 or 1, got '${SCOPY_LIBIIO_VERSION}'")
endif()

# v0 reuses the library/headers the parent Scopy build already resolved (same libiio as the rest of Scopy); fall back to
# a generic search for a standalone build. v1 is located by the iio-backend-v1 subdir itself.
if(SCOPY_LIBIIO_VERSION STREQUAL "0" AND (NOT IIO_LIBRARIES OR NOT IIO_INCLUDE_DIRS))
	find_library(IIO_LIBRARIES NAMES iio libiio REQUIRED)
	find_path(IIO_INCLUDE_DIRS NAMES iio.h REQUIRED)
endif()

# Tests needing a live libiio context; OFF so CI runs only hardware-free tests.
option(DC_ENABLE_HARDWARE_TESTS "Build device-controller tests that require a live libiio context" OFF)

add_subdirectory(core)
add_subdirectory(iio-utils)
add_subdirectory(iio-backend-v${SCOPY_LIBIIO_VERSION})
add_subdirectory(component)
