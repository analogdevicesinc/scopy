#
# Copyright (c) 2025 Analog Devices Inc.
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

cmake_minimum_required(VERSION 3.9)

set(SCOPY_MODULE m2k)
set(CURRENT_PKG_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(PACKAGE_NAME ${SCOPY_MODULE})
option(ENABLE_PLUGIN_M2K "Enable m2k plugin" ON)

message(STATUS "building package: " ${SCOPY_MODULE})

project(scopy-package-${SCOPY_MODULE} VERSION 0.1 LANGUAGES CXX)

configure_file(manifest.json.cmakein ${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/MANIFEST.json @ONLY)
message("Including plugins")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/plugins)
	if(NOT WITH_PYTHON)
		message(STATUS "Python is disabled or not found, M2K plugin disabled")
	endif()
	if(NOT WITH_SIGROK)
		message(STATUS "Libsigrokdecode is disabled or not found, M2K plugin disabled")
	endif()
	if(WITH_SIGROK AND WITH_PYTHON)
		add_plugins(${CMAKE_CURRENT_SOURCE_DIR}/plugins ${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/plugins)
	endif()
endif()

include_emu_xml(${CMAKE_CURRENT_SOURCE_DIR}/emu-xml ${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/emu-xml)
