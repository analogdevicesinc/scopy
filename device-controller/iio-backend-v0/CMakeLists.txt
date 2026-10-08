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

# iio-backend-v0 — libiio v0.x API backend. STATIC, linked into `component` (built when SCOPY_LIBIIO_VERSION=0); its
# extern "C" createIIOBackend() is resolved at link time.
if(NOT (IIO_LIBRARIES AND IIO_INCLUDE_DIRS))
	message(FATAL_ERROR "SCOPY_LIBIIO_VERSION=0 requires libiio v0 (IIO_LIBRARIES / IIO_INCLUDE_DIRS)")
endif()

file(GLOB V0_HEADERS include/v0/*.h)
file(GLOB V0_SOURCES src/*.cpp)

add_library(iio-backend-v0 STATIC ${V0_HEADERS} ${V0_SOURCES})

# Linked into the SHARED `component`, so it needs PIC.
set_target_properties(iio-backend-v0 PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(iio-backend-v0 PRIVATE include include/v0 ${IIO_INCLUDE_DIRS})

target_link_libraries(iio-backend-v0 PRIVATE Qt6::Core Qt6::Concurrent ${IIO_LIBRARIES} controller-iioutils)
