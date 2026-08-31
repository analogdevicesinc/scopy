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

# iio-backend-v0 — libiio v0 (legacy) backend.
#
# For the Scopy integration this is a STATIC library linked DIRECTLY into `component` (its extern "C" createIIOBackend()
# is resolved at link time; no dlopen). Building it as a SHARED dlopen plugin (libiio-backend-v0.so, for the dormant
# IIOBackendLoader path) is still possible via -DDC_BACKEND_V0_TYPE=SHARED.
if(NOT (IIO_LIBRARIES AND IIO_INCLUDE_DIRS))
	message(STATUS "libiio not found — skipping iio-backend-v0")
	return()
endif()

set(DC_BACKEND_V0_TYPE STATIC CACHE STRING
				    "iio-backend-v0 library type: STATIC (direct-link) or SHARED (dlopen plugin)"
)

file(GLOB V0_HEADERS include/v0/*.h)
file(GLOB V0_SOURCES src/*.cpp)

add_library(iio-backend-v0 ${DC_BACKEND_V0_TYPE} ${V0_HEADERS} ${V0_SOURCES})

# STATIC linked into a SHARED consumer needs PIC; harmless when SHARED. The PREFIX/ OUTPUT_NAME keep the plugin's exact
# libiio-backend-v0.so name when built SHARED.
set_target_properties(iio-backend-v0 PROPERTIES POSITION_INDEPENDENT_CODE ON PREFIX "lib" OUTPUT_NAME "iio-backend-v0")

target_include_directories(iio-backend-v0 PRIVATE include include/v0 ${IIO_INCLUDE_DIRS})

target_link_libraries(iio-backend-v0 PRIVATE Qt6::Core Qt6::Concurrent ${IIO_LIBRARIES} controller-iioutils)
