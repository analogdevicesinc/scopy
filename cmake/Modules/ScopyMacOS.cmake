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

foreach(plugin ${Qt5Gui_PLUGINS} ${Qt5Svg_PLUGINS})
	get_target_property(_loc ${plugin} LOCATION)
	get_filename_component(_name ${_loc} NAME)
	get_filename_component(_dir ${_loc} DIRECTORY)
	get_filename_component(_dir ${_dir} NAME)

	set_source_files_properties(${_loc} PROPERTIES MACOSX_PACKAGE_LOCATION plugins/${_dir})
	set(QT_PLUGINS ${QT_PLUGINS} ${_loc})
	set(BUNDLED_QT_PLUGINS ${BUNDLED_QT_PLUGINS} ${CMAKE_BINARY_DIR}/Scopy.app/Contents/plugins/${_dir}/${_name})
endforeach()

# needs revising
install(
	CODE "
	set(BU_CHMOD_BUNDLE_ITEMS ON)
	include(BundleUtilities)
	fixup_bundle(\"${CMAKE_BINARY_DIR}/Scopy.app\" \"${BUNDLED_QT_PLUGINS}\" \"${CMAKE_SOURCE_DIR}\")"
)

set(OSX_BUNDLE MACOSX_BUNDLE)

find_package(PkgConfig)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(LIBSIGROK_DECODE REQUIRED libsigrokdecode)
pkg_get_variable(LIBSIGROK_DECODERS_DIR libsigrokdecode decodersdir)

macro(set_macosx_package_location source_dir location extension)
	file(GLOB_RECURSE files ${source_dir}/*${extension})
	foreach(_file ${files})
		file(RELATIVE_PATH _relative_path ${source_dir} ${_file})
		get_filename_component(parent_directory ${_relative_path} DIRECTORY)
		message(parent_directory: ${parent_directory})
		set_property(SOURCE ${_file} PROPERTY MACOSX_PACKAGE_LOCATION ${location}/${parent_directory})
		message(location/parent_directory ${location}/${parent_directory})
		set(EXTRA_BUNDLE_FILES ${EXTRA_BUNDLE_FILES} ${_file})
	endforeach()
endmacro()

set_macosx_package_location(${LIBSIGROK_DECODERS_DIR} "MacOS/decoders" "py")
set_macosx_package_location(${CMAKE_BINARY_DIR}/style "MacOS/style" "")

set(EXTRA_BUNDLE_FILES
    ${EXTRA_BUNDLE_FILES}
    ${QT_PLUGINS}
    ${ICON_FILE}
    ${PKGINFO}
    ${QT_CONF}
)
