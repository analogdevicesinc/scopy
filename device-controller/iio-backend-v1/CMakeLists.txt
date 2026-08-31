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

# iio-backend-v1 — libiio v1 (new API) plugin. dlopen'd at runtime by IIOBackendLoader, which looks for
# "libiio-backend-v1.so". Built only when libiio.so.1 is present.
if(NOT (IIO_V1_LIB AND IIO_V1_INCLUDE))
	message(STATUS "libiio v1 not found — skipping iio-backend-v1")
	return()
endif()

file(GLOB V1_HEADERS include/v1/*.h)
file(GLOB V1_SOURCES src/*.cpp)

add_library(iio-backend-v1 SHARED ${V1_HEADERS} ${V1_SOURCES})

# Produce exactly libiio-backend-v1.so so the loader's dlopen("libiio-backend-v1.so") resolves it.
set_target_properties(iio-backend-v1 PROPERTIES PREFIX "lib" OUTPUT_NAME "iio-backend-v1")

target_include_directories(iio-backend-v1 PRIVATE include include/v1 ${IIO_V1_INCLUDE})

target_link_libraries(iio-backend-v1 PRIVATE Qt6::Core Qt6::Concurrent ${IIO_V1_LIB} controller-iioutils)
