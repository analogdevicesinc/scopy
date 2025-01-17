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

if(DEFINED __INCLUDED_SCOPY_STYLE_CMAKE)
	return()
endif()
set(__INCLUDED_SCOPY_STYLE_CMAKE TRUE)

# option can be --core or --plugin
function(generate_style option style_folder headers_folder)
	find_package(Python3 COMPONENTS Interpreter)
	execute_process(
		COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/style_generator.py ${option} ${style_folder}
			${headers_folder} ${CMAKE_BINARY_DIR} RESULT_VARIABLE ret
	)
	if(NOT ret EQUAL "0")
		message(FATAL_ERROR "Failed to generate style files! error: ${ret}")
	else()
		message("-- Generated style files")
	endif()
endfunction()
