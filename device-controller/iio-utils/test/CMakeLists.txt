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

# Per-interface backend conformance tests. Each is data-driven over whichever libiio backends load (v0/v1) and skips
# cleanly without a live context. Set DC_TEST_URI to a reachable context to exercise the live paths.
macro(add_backend_test name)
	add_executable(${name} ${name}.cpp backendfixture.h)
	target_link_libraries(${name} PRIVATE controller-iioutils Qt6::Test)
	# The backends are dlopen'd at runtime; ensure the plugins are built first and that the plugin dir is on the
	# executable's rpath so dlopen resolves them.
	set_target_properties(${name} PROPERTIES BUILD_RPATH "${DC_PLUGIN_DIR}")
	if(TARGET iio-backend-v0)
		add_dependencies(${name} iio-backend-v0)
	endif()
	if(TARGET iio-backend-v1)
		add_dependencies(${name} iio-backend-v1)
	endif()
	add_test(NAME ${name} COMMAND ${name})
endmacro()

add_backend_test(tst_scanops)
add_backend_test(tst_contextops)
add_backend_test(tst_deviceops)
add_backend_test(tst_channelops)
add_backend_test(tst_attrops)
add_backend_test(tst_bufferops)
