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

# iio-backend-v1 — libiio v1.x API backend. STATIC, linked into `component` (built when SCOPY_LIBIIO_VERSION=1); its
# extern "C" createIIOBackend() is resolved at link time.
#
# libiio v1 is located on its own: the root IIO_LIBRARIES / IIO_INCLUDE_DIRS point at the v0 API (the LIBIIO_COMPAT
# shim) used by the rest of Scopy. Set IIO_V1_LIBRARY / IIO_V1_INCLUDE_DIR to override.
find_path(IIO_V1_INCLUDE_DIR NAMES iio/iio.h)
find_library(IIO_V1_LIBRARY NAMES iio1 libiio1 iio libiio)
if(NOT (IIO_V1_LIBRARY AND IIO_V1_INCLUDE_DIR))
	message(FATAL_ERROR "SCOPY_LIBIIO_VERSION=1 requires libiio v1 (iio/iio.h + library); "
			    "set IIO_V1_INCLUDE_DIR / IIO_V1_LIBRARY"
	)
endif()

# `iio` may also name the v0 / compat library; make sure the one found really exports the v1 API. iio_create_context is
# the v1 entry point (v0 only has iio_create_context_from_uri).
include(CheckLibraryExists)
check_library_exists(${IIO_V1_LIBRARY} iio_create_context "" IIO_V1_LIBRARY_IS_V1)
if(NOT IIO_V1_LIBRARY_IS_V1)
	message(FATAL_ERROR "${IIO_V1_LIBRARY} is not libiio v1 (no iio_create_context); set IIO_V1_LIBRARY")
endif()
message(STATUS "device-controller libiio v1: ${IIO_V1_LIBRARY} (include: ${IIO_V1_INCLUDE_DIR})")

file(GLOB V1_HEADERS include/v1/*.h)
file(GLOB V1_SOURCES src/*.cpp)

add_library(iio-backend-v1 STATIC ${V1_HEADERS} ${V1_SOURCES})

# Linked into the SHARED `component`, so it needs PIC.
set_target_properties(iio-backend-v1 PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(iio-backend-v1 PRIVATE include include/v1 ${IIO_V1_INCLUDE_DIR})

target_link_libraries(iio-backend-v1 PRIVATE Qt6::Core Qt6::Concurrent ${IIO_V1_LIBRARY} controller-iioutils)
