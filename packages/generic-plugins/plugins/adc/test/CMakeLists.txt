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

cmake_minimum_required(VERSION 3.9)

include(ScopyTest)

setup_scopy_tests(pluginloader)

# The plot/channel classes are compiled *into* this test rather than linked from the plugin: the plugin is built with
# hidden visibility and none of them carries an export macro, so the symbols are not reachable from outside
# libscopy-adc.so. Exporting them just to be testable would widen the plugin's ABI for no other caller.
file(GLOB ACQ_TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../src/acq/acq*.cpp)
list(
	FILTER
	ACQ_TEST_SRC
	EXCLUDE
	REGEX
	"acqinstrument"
)
# The measurement/stats machinery and the digital track item, which the curve and digital kinds construct directly. Not
# part of src/acq, and both are plain classes rather than plugin entry points, so compiling them in costs nothing.
list(
	APPEND
	ACQ_TEST_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/../src/measurementcontroller.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../src/measure.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../src/sim/DigitalCurveItem.cpp
)
add_executable(${PROJECT_NAME}_test_acqchanneldelete tst_acqchanneldelete.cpp ${ACQ_TEST_SRC})
add_test(NAME ${PROJECT_NAME}_test_acqchanneldelete COMMAND ${PROJECT_NAME}_test_acqchanneldelete)
# The same set the plugin gives its own sources, plus include/${SCOPY_MODULE} for the generated scopy-adc_export.h and
# src/time for DigitalCurveItem.h.
target_include_directories(
	${PROJECT_NAME}_test_acqchanneldelete
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/acq
		${CMAKE_CURRENT_SOURCE_DIR}/../src
		${CMAKE_CURRENT_SOURCE_DIR}/../src/time
		${CMAKE_CURRENT_SOURCE_DIR}/../src/sim
		${CMAKE_CURRENT_SOURCE_DIR}/../include/${SCOPY_MODULE}
)
target_link_libraries(
	${PROJECT_NAME}_test_acqchanneldelete
	Qt${QT_VERSION_MAJOR}::Test
	scopy-gui
	scopy-core
	scopy-pluginbase
)
