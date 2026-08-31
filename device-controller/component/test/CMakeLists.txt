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

find_package(Qt6 REQUIRED COMPONENTS Test)

macro(add_component_test name)
	add_executable(${name} ${name}.cpp)
	target_link_libraries(${name} PRIVATE component controller-core controller-iioutils Qt6::Test)
	add_test(NAME ${name} COMMAND ${name})
endmacro()

add_component_test(tst_component_attribute)
add_component_test(tst_component_controller)
add_component_test(tst_component_stream)
add_component_test(tst_component_register)
add_component_test(tst_component_trigger)
