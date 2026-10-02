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

cmake_minimum_required(VERSION 3.16)
include(ScopyTest)

setup_scopy_tests(pluginloader)

# Compiled into the test rather than linked: the plugin is built with hidden visibility.
set(ACQ_TEST_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/acqannotationchannel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/acqaxis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/acqchannel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/acqchannelregistry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/acqcurvechannel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/acqdigitalchannel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/acqplot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/acqplotkind.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/acqplotmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/acqplotrail.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/acqtriggermarkerview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/acqwaterfallchannel.cpp
)
add_executable(${PROJECT_NAME}_test_acqchanneldelete tst_acqchanneldelete.cpp ${ACQ_TEST_SRC})
add_test(NAME ${PROJECT_NAME}_test_acqchanneldelete COMMAND ${PROJECT_NAME}_test_acqchanneldelete)
target_include_directories(
	${PROJECT_NAME}_test_acqchanneldelete PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
						      ${CMAKE_CURRENT_SOURCE_DIR}/../include/${SCOPY_MODULE}
)
target_link_libraries(
	${PROJECT_NAME}_test_acqchanneldelete
	Qt${QT_VERSION_MAJOR}::Test
	scopy-gui
	scopy-core
	scopy-pluginbase
)
