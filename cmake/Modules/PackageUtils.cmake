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

function(include_resources base_dir output_dir)
	if(NOT EXISTS ${base_dir})
		message(STATUS "Resources directory doesn't exist: ${base_dir}")
		return()
	endif()
	file(GLOB children RELATIVE ${base_dir} ${base_dir}/*)
	foreach(child IN LISTS children)
		# Skip "translations" directory
		if(child STREQUAL "translations")
			message(STATUS "Skipping excluded directory: ${child}")
			continue()
		endif()
		# Check if it's a directory
		if(IS_DIRECTORY ${base_dir}/${child})
			message(STATUS "Copying: ${child}")
			file(COPY ${base_dir}/${child} DESTINATION ${output_dir})
		endif()
	endforeach()
endfunction()

function(install_plugins SRC_DIR INSTALL_DEST TARGET_PREFIX)
	file(GLOB PLUGIN_DIRS RELATIVE ${SRC_DIR} ${SRC_DIR}/*)
	foreach(plugin ${PLUGIN_DIRS})
		set(target_name "${TARGET_PREFIX}-${plugin}")
		if(TARGET ${target_name})
			install(
				TARGETS ${target_name}
				RUNTIME DESTINATION ${INSTALL_DEST}
				LIBRARY DESTINATION ${INSTALL_DEST}
				ARCHIVE DESTINATION ${INSTALL_DEST}
			)
		else()
			message(STATUS "Skipping install for ${target_name} (target not defined)")
		endif()
		set(PLUGIN_DIR ${SRC_DIR}/${plugin})
		if(IS_DIRECTORY ${PLUGIN_DIR})
			set(RESOURCES_DIR ${PLUGIN_DIR}/resources)
			if(EXISTS ${RESOURCES_DIR})
				file(GLOB RESOURCES_CONTENT RELATIVE ${RESOURCES_DIR} ${RESOURCES_DIR}/*)
				list(REMOVE_ITEM RESOURCES_CONTENT "resources.qrc.depends")
				if(RESOURCES_CONTENT)
					message(STATUS "Installing resources: ${RESOURCES_CONTENT}")
					install(DIRECTORY ${RESOURCES_DIR} DESTINATION ${INSTALL_DEST}/${plugin}
						PATTERN "resources.qrc.depends" EXCLUDE
					)
				else()
					message(STATUS "No resources to install (only resources.qrc.depends found).")
				endif()
			endif()
		endif()
	endforeach()
endfunction()

function(install_pkg pkg_src_dir install_dir)
	install(FILES ${pkg_src_dir}/MANIFEST.json DESTINATION ${install_dir})
	if(EXISTS ${pkg_src_dir}/emu-xml)
		install(DIRECTORY ${pkg_src_dir}/emu-xml DESTINATION ${install_dir})
	endif()
	if(EXISTS ${pkg_src_dir}/style)
		install(DIRECTORY ${pkg_src_dir}/style DESTINATION ${install_dir})
	endif()
	if(EXISTS ${pkg_src_dir}/translations)
		install(DIRECTORY ${pkg_src_dir}/translations DESTINATION ${install_dir})
	endif()
	if(EXISTS ${pkg_src_dir}/resources)
		install(DIRECTORY ${pkg_src_dir}/resources DESTINATION ${install_dir})
	endif()
endfunction()
