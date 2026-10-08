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

# cmake/Modules/ScopyPackageDeps.cmake

set(SCOPY_PKG_DEPENDENCIES_DIR_NAME "dependencies")

function(scopy_package_dependency DEP_LABEL)
	cmake_parse_arguments(
		PKGDEP
		""
		"TARGET;HEADER"
		"LIB_NAMES"
		${ARGN}
	)

	set(_deps ${CMAKE_CURRENT_SOURCE_DIR}/${SCOPY_PKG_DEPENDENCIES_DIR_NAME})
	set(_staged ${SCOPY_PACKAGE_BUILD_PATH}/${SCOPY_MODULE}/${SCOPY_PKG_DEPENDENCIES_DIR_NAME})

	list(PREPEND CMAKE_PREFIX_PATH ${_deps})
	unset(_PKGDEP_LIB CACHE)
	unset(_PKGDEP_INC CACHE)
	find_library(_PKGDEP_LIB NAMES ${PKGDEP_LIB_NAMES})
	find_path(_PKGDEP_INC NAMES ${PKGDEP_HEADER})

	if(NOT _PKGDEP_LIB OR NOT _PKGDEP_INC)
		message(
			FATAL_ERROR
				"${SCOPY_MODULE}: dependency '${DEP_LABEL}' not found.\n"
				"  package-local prefix (searched first): ${_deps}\n"
				"    expected ${_deps}/lib/ and ${_deps}/include/${PKGDEP_HEADER}\n"
				"  system prefixes (searched second): ${PKGDEP_LIB_NAMES} / ${PKGDEP_HEADER}\n"
				"Place a prebuilt library and its headers in the package-local prefix, or install it system-wide.\n"
		)
	endif()

	string(FIND "${_PKGDEP_LIB}" "${_deps}/" _pos)
	if(_pos EQUAL 0)
		file(COPY ${_deps}/lib/ DESTINATION ${_staged})
		get_filename_component(_lib_name ${_PKGDEP_LIB} NAME)
		set(_link_lib ${_staged}/${_lib_name})

		install(DIRECTORY ${_staged}/
			DESTINATION ${SCOPY_PACKAGE_INSTALL_PATH}/${SCOPY_MODULE}/${SCOPY_PKG_DEPENDENCIES_DIR_NAME}
		)
		list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN/../${SCOPY_PKG_DEPENDENCIES_DIR_NAME}")
		list(APPEND CMAKE_BUILD_RPATH "$ORIGIN/../${SCOPY_PKG_DEPENDENCIES_DIR_NAME}")
		set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}" PARENT_SCOPE)
		set(CMAKE_BUILD_RPATH "${CMAKE_BUILD_RPATH}" PARENT_SCOPE)
		add_link_options(-Wl,--disable-new-dtags)

		set(_from PACKAGE)
	else()
		set(_link_lib ${_PKGDEP_LIB})
		set(_from SYSTEM)
	endif()

	add_library(${PKGDEP_TARGET} SHARED IMPORTED GLOBAL)
	set_target_properties(
		${PKGDEP_TARGET} PROPERTIES IMPORTED_LOCATION ${_link_lib} INTERFACE_INCLUDE_DIRECTORIES ${_PKGDEP_INC}
	)
	message(STATUS "${SCOPY_MODULE}: dependency '${DEP_LABEL}' resolved from ${_from} -> ${_link_lib}")
endfunction()
