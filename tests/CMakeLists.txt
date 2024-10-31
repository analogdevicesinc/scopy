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

cmake_minimum_required(VERSION 3.5)

# JS AUTOMATED TESTS
option(ENABLE_AUTOMATED_TESTS "Enable JS Automated Tests" OFF)

if(ENABLE_AUTOMATED_TESTS)
	# Set build path as CMAKE CACHE variable to use in tests
	set(SCOPY_BUILD_PATH ${CMAKE_BINARY_DIR} CACHE STRING "SCOPY_BUILD_PATH" FORCE)

	# SCOPY GENERIC TESTS
	add_test(NAME "ScopyDefaultJSTests"
		 COMMAND bash ${CMAKE_SOURCE_DIR}/js/test.sh "${CMAKE_SOURCE_DIR}/resources/emu_xml/"
			 "${CMAKE_SOURCE_DIR}/js/scopyDefaultTests.js"
	)

	# PLUGIN SPECIFIC TESTS

	# DATALOGGER
	if(ENABLE_PLUGIN_DATALOGGER)
		add_test(NAME "DataloggerLogAndLoadJSTests"
			 COMMAND bash ${CMAKE_SOURCE_DIR}/js/test.sh "${CMAKE_SOURCE_DIR}/resources/emu_xml/"
				 "${CMAKE_SOURCE_DIR}/plugins/datalogger/js/dataLoggerAutomatedTest.js"
		)
	endif()

	# REGMAP
	if(ENABLE_PLUGIN_REGMAP)
		add_test(NAME "RegMapJSTests"
			 COMMAND bash ${CMAKE_SOURCE_DIR}/js/test.sh "${CMAKE_SOURCE_DIR}/resources/emuXml/"
				 "${CMAKE_SOURCE_DIR}/plugins/regmap/js/regMapAutomatedTest.js"
		)
	endif()

endif()
