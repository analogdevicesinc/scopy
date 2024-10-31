#
# Copyright (c) 2024 Analog Devices Inc.
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

# * try to find Qwt libraries and include files QWT_INCLUDE_DIR where to find qwt_global.h, etc. QWT_LIBRARIES libraries
#   to link against QWT_FOUND If false, do not try to use Qwt qwt_global.h holds a string with the QWT version; test to
#   make sure it's at least 5.2

find_path(
	QWT_INCLUDE_DIRS
	NAMES qwt_global.h
	HINTS ${CMAKE_INSTALL_PREFIX}/include/qwt
	PATHS /usr/local/include/qwt-qt5
	      /usr/local/include/qwt
	      /usr/include/qwt6
	      /usr/include/qwt-qt5
	      /usr/include/qwt
	      /usr/include/qwt5
	      /opt/local/include/qwt
	      /sw/include/qwt
	      /usr/local/lib/qwt.framework/Headers
)

find_library(
	QWT_LIBRARIES
	NAMES qwt6 qwt6-qt5 qwt qwt-qt5
	HINTS ${CMAKE_INSTALL_PREFIX}/lib ${CMAKE_INSTALL_PREFIX}/lib64
	PATHS /usr/local/lib
	      /usr/lib
	      /opt/local/lib
	      /sw/lib
	      /usr/local/lib/qwt.framework
)

set(QWT_FOUND FALSE)
if(QWT_INCLUDE_DIRS)
	file(STRINGS "${QWT_INCLUDE_DIRS}/qwt_global.h" QWT_STRING_VERSION REGEX "QWT_VERSION_STR")
	set(QWT_WRONG_VERSION True)
	set(QWT_VERSION "No Version")
	string(REGEX MATCH "[0-9]+.[0-9]+.[0-9]+" QWT_VERSION ${QWT_STRING_VERSION})
	string(COMPARE LESS ${QWT_VERSION} "5.2.0" QWT_WRONG_VERSION)
	string(COMPARE GREATER ${QWT_VERSION} "6.2.0" QWT_WRONG_VERSION)

	message(STATUS "QWT Version: ${QWT_VERSION}")
	if(NOT QWT_WRONG_VERSION)
		set(QWT_FOUND TRUE)
	else(NOT QWT_WRONG_VERSION)
		message(STATUS "QWT Version must be >= 5.2 and <= 6.2.0, Found ${QWT_VERSION}")
	endif(NOT QWT_WRONG_VERSION)

endif(QWT_INCLUDE_DIRS)

if(QWT_FOUND)
	# handle the QUIETLY and REQUIRED arguments and set QWT_FOUND to TRUE if all listed variables are TRUE
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(Qwt DEFAULT_MSG QWT_LIBRARIES QWT_INCLUDE_DIRS)
	mark_as_advanced(QWT_LIBRARIES QWT_INCLUDE_DIRS)
endif(QWT_FOUND)
