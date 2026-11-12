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

# set(CMAKE_FIND_DEBUG_MODE ON)
include(ScopyTest)

setup_scopy_tests(pluginloader)

# The access layer and the data store, against fake strategies
set(ACCESS_TEST_TARGET ${PROJECT_NAME}_test_registermapaccess)
add_executable(
	${ACCESS_TEST_TARGET} tst_registermapaccess.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/registermapaccess.cpp
			      ${CMAKE_CURRENT_SOURCE_DIR}/../src/registermapdatastore.cpp
			      ${CMAKE_CURRENT_SOURCE_DIR}/../src/logging_categories.cpp
)
target_include_directories(
	${ACCESS_TEST_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/../include/regmap
)
target_link_libraries(${ACCESS_TEST_TARGET} Qt${QT_VERSION_MAJOR}::Test component scopy-pluginbase)
add_test(NAME ${ACCESS_TEST_TARGET} COMMAND ${ACCESS_TEST_TARGET})
