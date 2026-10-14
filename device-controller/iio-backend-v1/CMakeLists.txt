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
# libiio v1 is located on its own (the root IIO_LIBRARIES / IIO_INCLUDE_DIRS are only set for v0 builds); this backend
# is the only part of Scopy that uses libiio in a v1 build. Pass -DIIO_V1_INCLUDE_DIR / -DIIO_V1_LIBRARY to override the
# auto-detection below.
#
# The v1 header lives at <prefix>/include/iio/iio.h (unambiguously v1 — v0 has a flat <prefix>/include/iio.h). Find it
# first, then look for the library NEXT TO it (same prefix) with NO_DEFAULT_PATH, so a v0 libiio.so elsewhere on the
# system (e.g. /usr/local) can't shadow the real v1. Fall back to a default search only if that misses.
find_path(IIO_V1_INCLUDE_DIR NAMES iio/iio.h)
if(IIO_V1_INCLUDE_DIR AND NOT IIO_V1_LIBRARY)
	get_filename_component(_iio_v1_prefix "${IIO_V1_INCLUDE_DIR}" DIRECTORY) # strip trailing /include
	find_library(
		IIO_V1_LIBRARY
		NAMES iio1 libiio1 iio libiio
		HINTS ${_iio_v1_prefix}/lib ${_iio_v1_prefix}/lib/${CMAKE_LIBRARY_ARCHITECTURE}
		NO_DEFAULT_PATH
	)
endif()
find_library(IIO_V1_LIBRARY NAMES iio1 libiio1 iio libiio) # fallback (also a no-op if already found)
if(NOT (IIO_V1_LIBRARY AND IIO_V1_INCLUDE_DIR))
	message(FATAL_ERROR "SCOPY_LIBIIO_VERSION=1 requires libiio v1 (iio/iio.h + library); "
			    "set IIO_V1_INCLUDE_DIR / IIO_V1_LIBRARY"
	)
endif()

# `iio` may also name the v0 / compat library; make sure the one found really exports the v1 API. iio_create_context is
# the v1 entry point (v0 only has iio_create_context_from_uri). On mismatch, clear the cached path so the next configure
# re-detects instead of getting stuck on the bad value.
include(CheckLibraryExists)
check_library_exists(${IIO_V1_LIBRARY} iio_create_context "" IIO_V1_LIBRARY_IS_V1)
if(NOT IIO_V1_LIBRARY_IS_V1)
	set(_bad_v1 "${IIO_V1_LIBRARY}")
	unset(IIO_V1_LIBRARY CACHE)
	message(FATAL_ERROR "${_bad_v1} is not libiio v1 (no iio_create_context). "
			    "Pass -DIIO_V1_LIBRARY=/path/to/libiio.so.1 (and -DIIO_V1_INCLUDE_DIR)."
	)
endif()
message(STATUS "device-controller libiio v1: ${IIO_V1_LIBRARY} (include: ${IIO_V1_INCLUDE_DIR})")

file(GLOB V1_HEADERS include/v1/*.h)
file(GLOB V1_SOURCES src/*.cpp)

add_library(iio-backend-v1 STATIC ${V1_HEADERS} ${V1_SOURCES})

# Linked into the SHARED `component`, so it needs PIC.
set_target_properties(iio-backend-v1 PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(iio-backend-v1 PRIVATE include include/v1 ${IIO_V1_INCLUDE_DIR})

target_link_libraries(iio-backend-v1 PRIVATE Qt6::Core Qt6::Concurrent ${IIO_V1_LIBRARY} controller-iioutils)
