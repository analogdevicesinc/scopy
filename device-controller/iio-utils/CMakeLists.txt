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

# iio-utils — libiio backend interfaces (I*Ops), opaque handles, concrete commands, the dlopen backend loader, and the
# scan helper. Depends on core; no direct libiio.
file(GLOB IIOUTIL_HEADERS include/iioutil/*.h include/iioutil/commands/*.h)
file(GLOB IIOUTIL_SOURCES src/*.cpp)

# Test purpose only (it is not cross platform)
list(REMOVE_ITEM IIOUTIL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/iiobackendloader.cpp)

add_library(controller-iioutils SHARED ${IIOUTIL_HEADERS} ${IIOUTIL_SOURCES})

set_target_properties(controller-iioutils PROPERTIES OUTPUT_NAME scopy-controller-iioutils)

target_include_directories(controller-iioutils PUBLIC include)

target_link_libraries(controller-iioutils PUBLIC controller-core Qt6::Core)

install(TARGETS controller-iioutils LIBRARY DESTINATION ${SCOPY_DLL_INSTALL_PATH} COMPONENT ${SCOPY_PDK}
	RUNTIME DESTINATION ${SCOPY_DLL_INSTALL_PATH}
)
