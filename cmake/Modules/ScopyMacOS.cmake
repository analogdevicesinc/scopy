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

if(DEFINED __INCLUDED_SCOPY_MACOS_CMAKE)
	return()
endif()
set(__INCLUDED_SCOPY_MACOS_CMAKE TRUE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

set(PKGINFO ${CMAKE_BINARY_DIR}/PkgInfo)
file(WRITE ${PKGINFO} "APPLScopy")
set_source_files_properties(${PKGINFO} PROPERTIES MACOSX_PACKAGE_LOCATION .)

set(QT_CONF ${CMAKE_BINARY_DIR}/qt.conf)
file(APPEND ${QT_CONF} "")
set_source_files_properties(${QT_CONF} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

set(ICON_FILE ${CMAKE_SOURCE_DIR}/gui/res/Scopy.icns)
set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

set(CMAKE_EXE_LINKER_FLAGS "-Wl,-headerpad_max_install_names -Wl,-search_paths_first ${CMAKE_EXE_LINKER_FLAGS}")

set(OSX_BUNDLE MACOSX_BUNDLE)

find_package(PkgConfig)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(LIBSIGROK_DECODE REQUIRED libsigrokdecode)
pkg_get_variable(LIBSIGROK_DECODERS_DIR libsigrokdecode decodersdir)

set(EXTRA_BUNDLE_FILES ${ICON_FILE} ${PKGINFO} ${QT_CONF})

function(setup_macos_bundle_resources target)
	add_custom_command(
		TARGET ${target}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/style
			$<TARGET_BUNDLE_DIR:${target}>/Contents/Resources/style
		COMMENT "Copying style files into app bundle"
	)

	add_custom_command(
		TARGET ${target}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIBSIGROK_DECODERS_DIR}
			$<TARGET_BUNDLE_DIR:${target}>/Contents/Resources/decoders
		COMMENT "Copying sigrokdecode decoders into app bundle"
	)

	add_custom_command(
		TARGET ${target}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/translations
			$<TARGET_BUNDLE_DIR:${target}>/Contents/Resources/translations
		COMMENT "Copying translations into app bundle"
	)
endfunction()
