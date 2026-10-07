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

# Local libm2k dependency shim for the adalm2000 package.
#
# Loaded via include() from the plugin CMakeLists, so this file executes in the same cmake scope as the caller — no
# PARENT_SCOPE or GLOBAL needed.
#
# To use: drop a pre-built libm2k into the sibling lib/ and include/ directories:
# packages/adalm2000/dependencies/lib/libm2k.so packages/adalm2000/dependencies/include/libm2k/  (all headers)
#
# CMAKE_CURRENT_LIST_DIR = .../packages/adalm2000/dependencies/cmake/libm2k/ Two levels up           =
# .../packages/adalm2000/dependencies/
get_filename_component(_ADALM2000_DEPS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(_ADALM2000_LIB "${_ADALM2000_DEPS_ROOT}/lib/libm2k.so")
set(_ADALM2000_INC "${_ADALM2000_DEPS_ROOT}/include")

if(NOT EXISTS "${_ADALM2000_LIB}")
	set(libm2k_FOUND FALSE)
	return()
endif()

if(NOT TARGET libm2k::libm2k)
	add_library(libm2k::libm2k SHARED IMPORTED)
	set_target_properties(
		libm2k::libm2k
		PROPERTIES IMPORTED_LOCATION "${_ADALM2000_LIB}" INTERFACE_INCLUDE_DIRECTORIES "${_ADALM2000_INC}"
			   INTERFACE_LINK_LIBRARIES ""
	)
endif()

set(libm2k_FOUND TRUE)
message(STATUS "adalm2000: using local bundled libm2k from ${_ADALM2000_DEPS_ROOT}/lib/")
