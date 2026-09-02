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

# The shared helper, as every other suite in the tree uses. The local SETUP_TESTS this
# replaces looped over ${ARGS} — never set, since the parameter is ${ARGN} — so its body
# never ran and nothing here was ever built or registered with ctest.
include(ScopyTest)

# tst_test1.cpp is deliberately not listed: it is a license header with no test in it.
setup_scopy_tests(waterfalldata)
