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

# cmake/PackageUtils.cmake

function(add_subdirectories_with_options base_dir output_dir prefix)
	file(GLOB CHILDREN RELATIVE ${base_dir} ${base_dir}/*)

	foreach(child ${CHILDREN})
		if(IS_DIRECTORY ${base_dir}/${child})
			string(TOUPPER ${child} CHILD_UPPER)
			set(ENABLE_CHILD_VAR "${prefix}_${CHILD_UPPER}")

			# Set default ON
			if(NOT DEFINED ${ENABLE_CHILD_VAR})
				set(${ENABLE_CHILD_VAR} ON CACHE BOOL "Enable ${prefix} ${child}")
			endif()

			if(${ENABLE_CHILD_VAR})
				message(STATUS "Including ${prefix}: ${child}")

				# Set output directories based on platform
				if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
					set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${output_dir})
				elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
					set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
					    "${CMAKE_BINARY_DIR}/Scopy.app/Contents/MacOS/${output_dir}"
					)
				else()
					set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${output_dir})
				endif()

				if(EXISTS "${base_dir}/${child}/CMakeLists.txt")
					add_subdirectory(${base_dir}/${child})
				else()
					message(STATUS "No CMakeLists.txt in subdirectory: ${base_dir}/${child}")
				endif()
			else()
				message(STATUS "Skipping ${prefix}: ${child}")
			endif()
		endif()
	endforeach()
endfunction()

function(enable_all base_dir enable prefix)
	file(GLOB CHILDREN RELATIVE ${base_dir} ${base_dir}/*)
	foreach(child ${CHILDREN})
		if(IS_DIRECTORY ${base_dir}/${child})
			string(TOUPPER ${child} CHILD_UPPER)
			set(ENABLE_CHILD_VAR "${prefix}_${CHILD_UPPER}")

			if(NOT DEFINED ${ENABLE_CHILD_VAR})
				set(${ENABLE_CHILD_VAR} ${enable} CACHE BOOL "Enable ${prefix} ${child}")
			endif()
		endif()
	endforeach()
endfunction()

function(enable_all_packages base_dir enable)
	enable_all(${base_dir} ${enable} "ENABLE_PACKAGE")
endfunction()

function(add_packages base_dir output_dir)
	add_subdirectories_with_options(${base_dir} ${output_dir} "ENABLE_PACKAGE")
endfunction()

function(add_plugins base_dir output_dir)
	add_subdirectories_with_options(${base_dir} ${output_dir} "ENABLE_PLUGIN")
endfunction()

function(include_emu_xml base_dir output_dir)
	if(IS_DIRECTORY ${base_dir})
		message("Found IIO Emulator XMLs in: " ${base_dir})
		file(GLOB EMU_FILES ${base_dir}/*)
		if(EMU_FILES)
			file(COPY ${EMU_FILES} DESTINATION ${output_dir})
		else()
			message("No emulator files found in: ${base_dir}")
		endif()
	endif()
endfunction()
