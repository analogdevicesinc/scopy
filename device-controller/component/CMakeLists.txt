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

# component — identity/capability component tree (Context/Device/Channel/Attribute), the Controller + backend context
# factories, and the IIO backend leaves. Depends on core and iio-utils.
file(GLOB COMPONENT_HEADERS include/component/*.h include/component/backends/iio/*.h)
file(GLOB COMPONENT_SOURCES src/*.cpp src/backends/iio/*.cpp)

add_library(component SHARED ${COMPONENT_HEADERS} ${COMPONENT_SOURCES})

target_include_directories(component PUBLIC include)

target_link_libraries(
	component
	PUBLIC controller-core
	       controller-iioutils
	       Qt6::Core
	       QCoro6::Core
	       QCoro6::Coro
	PRIVATE iio-backend-v0
)

# Direct-link libiio v0: IIOContextFactory instantiates the v0 backend via the statically-linked createIIOBackend()
# instead of dlopen'ing a plugin through IIOBackendLoader (which stays compiled but dormant). See iiocontextfactory.cpp.
target_compile_definitions(component PRIVATE SCOPY_DC_DIRECT_LIBIIO_V0)
