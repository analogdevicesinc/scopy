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

if(DEFINED __INCLUDED_SCOPY_WHATS_NEW_CMAKE)
	return()
endif()
set(__INCLUDED_SCOPY_WHATS_NEW_CMAKE TRUE)

function(generate_whats_new RESOURCE_FILE FOLDER)
	# Specify the folder containing resources Create the list of HTML files recursively

	file(GLOB_RECURSE HTML_FILES "${FOLDER}*.html")

	# Start writing to the resource file
	file(WRITE "${RESOURCE_FILE}" "<RCC>\n  <qresource prefix=\"/whatsnew/\">\n")

	# Loop through each HTML file and add it to the resource file
	foreach(HTML_FILE ${HTML_FILES})
		# Convert the file path to a resource path
		file(RELATIVE_PATH relative ${FOLDER} ${HTML_FILE})

		# Write the file to the resource file
		file(APPEND "${RESOURCE_FILE}" "    <file>${relative}</file>\n")
	endforeach()

	# End the resource file
	file(APPEND "${RESOURCE_FILE}" "  </qresource>\n</RCC>\n")
endfunction()
