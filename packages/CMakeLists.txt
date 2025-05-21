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

project(scopy-packages VERSION 0.1 LANGUAGES CXX)

# If DEV_MODE, remove all previously built packages
if(SCOPY_DEV_MODE)
	file(REMOVE_RECURSE ${SCOPY_PACKAGE_BUILD_PATH})
endif()

# Include utils
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include(ScopyWindowsInstaller)
endif()
include(PackageUtils)

if(DISABLE_ALL_PACKAGES)
	enable_all_packages(${CMAKE_CURRENT_SOURCE_DIR} OFF)
	message(STATUS "DISABLE_ALL_PACKAGES is set.")
endif()
add_packages(${CMAKE_CURRENT_SOURCE_DIR} ${SCOPY_PACKAGE_BUILD_PATH})
