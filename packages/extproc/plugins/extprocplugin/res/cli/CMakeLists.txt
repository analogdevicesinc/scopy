#
# Copyright (c) 2025 Analog Devices Inc.
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

project(cli_analyzer VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Create the executable with bundled cJSON
add_executable(cli_analyzer main.c cjson/cJSON.c)

# Set up include directories - cJSON.h is in the same directory
target_include_directories(cli_analyzer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(cli_analyzer PRIVATE -Wall -Wextra)

# Set output directory to be alongside scopy executable
set_target_properties(cli_analyzer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
