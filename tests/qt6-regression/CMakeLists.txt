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
project(qt6-regression-tests)

find_package(Qt6 COMPONENTS Test Core Concurrent Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# --- Standalone tests (no Scopy library deps) ---

# Regex patterns
add_executable(tst_regex_patterns tst_regex_patterns.cpp)
target_link_libraries(tst_regex_patterns Qt6::Test Qt6::Core)
add_test(NAME tst_regex_patterns COMMAND tst_regex_patterns)

# QString/QVariant
add_executable(tst_qstring_qvariant tst_qstring_qvariant.cpp)
target_link_libraries(tst_qstring_qvariant Qt6::Test Qt6::Core Qt6::Widgets)
add_test(NAME tst_qstring_qvariant COMMAND tst_qstring_qvariant)

# Signal/slot
add_executable(tst_signalslot_qt6 tst_signalslot_qt6.cpp)
target_link_libraries(tst_signalslot_qt6 Qt6::Test Qt6::Core Qt6::Concurrent)
add_test(NAME tst_signalslot_qt6 COMMAND tst_signalslot_qt6)

# --- Library-linked tests (require scopyiioutil) ---

# CommandQueue
add_executable(tst_commandqueue_qt6 tst_commandqueue_qt6.cpp)
target_link_libraries(
	tst_commandqueue_qt6
	Qt6::Test
	Qt6::Core
	Qt6::Concurrent
	Qt6::Widgets
	scopy-iioutil
)
target_include_directories(tst_commandqueue_qt6 PRIVATE ${CMAKE_SOURCE_DIR}/iioutil/include)
add_test(NAME tst_commandqueue_qt6 COMMAND tst_commandqueue_qt6)

# Threading (standalone — tests QThread lifecycle patterns without CyclicalTask's iio.h dep)
add_executable(tst_threading_qt6 tst_threading_qt6.cpp)
target_link_libraries(tst_threading_qt6 Qt6::Test Qt6::Core)
add_test(NAME tst_threading_qt6 COMMAND tst_threading_qt6)
