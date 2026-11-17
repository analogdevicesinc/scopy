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

# cmake/ScopyDependencyChecks.cmake

# Sets out_var to the libiio major version (0 or 1) the found libm2k was built against, or empty if libm2k is not found.
# libm2k exports LIBIIO_V1 as a PUBLIC compile definition when built against libiio v1.
function(scopy_get_libm2k_libiio_version out_var)
	set(${out_var} "" PARENT_SCOPE)
	find_package(libm2k QUIET)
	if(NOT TARGET libm2k::libm2k)
		return()
	endif()
	get_target_property(_defs libm2k::libm2k INTERFACE_COMPILE_DEFINITIONS)
	if("LIBIIO_V1" IN_LIST _defs)
		set(${out_var} 1 PARENT_SCOPE)
	else()
		set(${out_var} 0 PARENT_SCOPE)
	endif()
endfunction()

# Disables package pkg (ENABLE_PACKAGE_<PKG>) for this configure only when the found libm2k was built against a
# different libiio than SCOPY_LIBIIO_VERSION; mixing libiio v0 and v1 in one process crashes on connect. Sets a normal
# variable in the caller's scope that shadows the cache, so the package comes back once libm2k matches.
function(scopy_require_libm2k_libiio_match pkg)
	string(TOUPPER ${pkg} _pkg_upper)
	set(_enable_var ENABLE_PACKAGE_${_pkg_upper})
	if(DEFINED ${_enable_var} AND NOT ${${_enable_var}})
		return()
	endif()
	scopy_get_libm2k_libiio_version(_m2k_iio_version)
	if(_m2k_iio_version STREQUAL "" OR _m2k_iio_version STREQUAL SCOPY_LIBIIO_VERSION)
		return()
	endif()
	message(WARNING "Skipping ${pkg}: libm2k was built against libiio v${_m2k_iio_version} "
			"but SCOPY_LIBIIO_VERSION=${SCOPY_LIBIIO_VERSION}"
	)
	set(${_enable_var} OFF PARENT_SCOPE)
endfunction()
