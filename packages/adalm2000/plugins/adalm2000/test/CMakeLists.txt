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

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

# Unit tests compile the header-only code under test directly into the test binary: linking libscopy-adalm2000.so would
# resolve nothing, because the plugin is built with hidden visibility.
#
# Each call site names only the headers it needs (${ARGN}). The plugin-wide HEADER_LIST glob would feed every Q_OBJECT
# header -- including .cpp-backed ones like M2kContext -- to AUTOMOC in every test binary, causing undefined-reference
# failures for out-of-line destructors that include libm2k headers.
macro(add_adalm2000_unit_test name)
	set(_t ${PROJECT_NAME}_test_${name})
	add_executable(${_t} tst_${name}.cpp ${ARGN})
	target_include_directories(
		${_t}
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../include/${SCOPY_MODULE}
			${CMAKE_CURRENT_SOURCE_DIR}/../include/${SCOPY_MODULE}/m2kBackend
	)
	target_link_libraries(
		${_t}
		PRIVATE Qt${QT_VERSION_MAJOR}::Test
			Qt${QT_VERSION_MAJOR}::Core
			component
			controller-core
			QCoro6::Core
			QCoro6::Coro
	)
	add_test(NAME ${_t} COMMAND ${_t})
endmacro()

add_adalm2000_unit_test(m2kcallcommand)

add_adalm2000_unit_test(
	m2kattribute ../include/adalm2000/m2kBackend/m2kattributereader.h
	../include/adalm2000/m2kBackend/m2kattributewriter.h
)

add_adalm2000_unit_test(m2kping ../include/adalm2000/m2kBackend/m2kping.h)

setup_scopy_tests(pluginloader)
