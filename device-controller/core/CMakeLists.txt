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

# core — Result, Command, executors, BufferUser. No backend/libiio types, no Qt widgets.
file(GLOB CORE_HEADERS include/core/*.h)
file(GLOB CORE_SOURCES src/*.cpp)

add_library(controller-core SHARED ${CORE_HEADERS} ${CORE_SOURCES})

set_target_properties(controller-core PROPERTIES OUTPUT_NAME scopy-controller-core)

target_include_directories(controller-core PUBLIC include)

target_link_libraries(controller-core PUBLIC Qt6::Core Qt6::Concurrent QCoro6::Core QCoro6::Coro)

install(TARGETS controller-core LIBRARY DESTINATION ${SCOPY_DLL_INSTALL_PATH} COMPONENT ${SCOPY_PDK}
	RUNTIME DESTINATION ${SCOPY_DLL_INSTALL_PATH}
)
