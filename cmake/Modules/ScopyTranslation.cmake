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

if(DEFINED __INCLUDED_SCOPY_TRANSLATIONS_CMAKE)
	return()
endif()
set(__INCLUDED_SCOPY_TRANSLATIONS_CMAKE TRUE)

function(generate_translations)
	file(GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/translations/*.ts)
	set_source_files_properties(
		${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_BINARY_DIR}/translations
				       MACOSX_PACKAGE_LOCATION ${CMAKE_BINARY_DIR}/translations
	)
	qt_add_translation(QM_FILES ${TS_FILES})

	set(TRANSLATIONS)
	foreach(file ${TS_FILES})
		get_filename_component(file_name ${file} NAME_WE)
		set(TRANSLATIONS "${TRANSLATIONS}\n<file>${CMAKE_BINARY_DIR}/translations/${file_name}.qm</file>")
	endforeach()

	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/resources/translations.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
		@ONLY
	)
endfunction()
