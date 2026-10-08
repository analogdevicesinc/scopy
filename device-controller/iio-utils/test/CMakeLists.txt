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

# Per-interface backend conformance tests, run against the backend linked into this build
# (iio-backend-v${SCOPY_LIBIIO_VERSION}). They skip cleanly without a live context; set DC_TEST_URI to a reachable
# context to exercise the live paths.
macro(add_backend_test name)
	add_executable(${name} ${name}.cpp backendfixture.h)
	target_link_libraries(${name} PRIVATE controller-iioutils iio-backend-v${SCOPY_LIBIIO_VERSION} Qt6::Test)
	add_test(NAME ${name} COMMAND ${name})
endmacro()

add_backend_test(tst_scanops)
add_backend_test(tst_contextops)
add_backend_test(tst_deviceops)
add_backend_test(tst_channelops)
add_backend_test(tst_attrops)
add_backend_test(tst_bufferops)
