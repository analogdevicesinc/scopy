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

cmake_minimum_required(VERSION 3.16)
include(ScopyTest)
set(DCDEBUGGER_TESTS
    componenttree
    streamacquisition
    acquisitionui
    rawacquisitionview
    signalmath
    streamgeneration
    rawgeneratorview
    registers
    attributemonitor
    monitorview
    channelmonitorview
    commandlog
    pingview
    nodepanel
    nodeviews
    bottompanes
    codegenerator
    logscreenshots
)
setup_scopy_tests(${DCDEBUGGER_TESTS})
foreach(t ${DCDEBUGGER_TESTS})
	target_include_directories(
		${PROJECT_NAME}_test_${t} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
						  ${CMAKE_CURRENT_SOURCE_DIR}/../include/dcdebugger
	)
	# The style is looked up in "./style" of the build folder (SCOPY_STYLE_BUILD_PATH).
	target_compile_definitions(${PROJECT_NAME}_test_${t} PRIVATE DCDEBUGGER_TEST_BUILD_DIR="${CMAKE_BINARY_DIR}")
endforeach()
