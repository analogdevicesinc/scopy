cmake_minimum_required(VERSION 3.9)
project(scopy-plugins VERSION 0.1 LANGUAGES CXX)

message(STATUS "Plugins folder: " ${SCOPY_PLUGIN_BUILD_PATH})
message(STATUS "Plugins folder after install: " ${SCOPY_PLUGIN_INSTALL_PATH})

option(ENABLE_PLUGIN_M2K "Enable m2k plugin" ON)
option(ENABLE_PLUGIN_TEST "Enable test plugin" OFF)
option(ENABLE_PLUGIN_REGMAP "Enable regmap plugin" ON)
option(ENABLE_PLUGIN_DEBUGGER "Enable debugger plugin" ON)
option(ENABLE_PLUGIN_ADC "Enable ADC plugin" ON)
option(ENABLE_PLUGIN_SWIOT "Enable SWIOT plugin" ON)
option(ENABLE_PLUGIN_PQM "Enable PQM plugin" ON)
option(ENABLE_PLUGIN_DATALOGGER "Enable DATALOGGER plugin" ON)
option(ENABLE_PLUGIN_DAC "Enable DAC plugin" ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SCOPY_PLUGIN_BUILD_PATH})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Scopy.app/Contents/MacOS/plugins")
else()
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SCOPY_PLUGIN_BUILD_PATH})
endif()

# The order the plugins are added here will also affect the installer order. Make sure fixed plugin are before optional
# ones.

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include(ScopyWindowsInstaller)
endif()

if(ENABLE_PLUGIN_TEST)
	add_subdirectory(guitest)
	list(APPEND PLUGINS ${PLUGIN_NAME})
	add_subdirectory(test)
	list(APPEND PLUGINS ${PLUGIN_NAME})
	add_subdirectory(test2)
	list(APPEND PLUGINS ${PLUGIN_NAME})
	add_subdirectory(bareminimum)
	list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

# If in DEV_MODE remove all ".so" files on each build to ensure only ENABLED plugins have the ".so" file generated
if(SCOPY_DEV_MODE)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		file(GLOB DLL_FILES "${SCOPY_PLUGIN_BUILD_PATH}/*.dll")
		if(NOT DLL_FILES STREQUAL "")
			file(REMOVE ${DLL_FILES})
		endif()
	else()
		file(GLOB SO_FILES "${SCOPY_PLUGIN_BUILD_PATH}/*.so")
		if(NOT SO_FILES STREQUAL "")
			file(REMOVE ${SO_FILES})
		endif()
	endif()
endif()

if(ENABLE_PLUGIN_ADC)
	add_subdirectory(adc)
	list(APPEND PLUGINS ${ADC_TARGET_NAME})
endif()

if(ENABLE_PLUGIN_REGMAP)
	add_subdirectory(regmap)
	list(APPEND PLUGINS ${REGNMAP_TARGET_NAME})
endif()

if(ENABLE_PLUGIN_DEBUGGER)
	add_subdirectory(debugger)
	list(APPEND PLUGINS ${DEBUGGER_TARGET_NAME})
endif()

if(ENABLE_PLUGIN_DATALOGGER)
	add_subdirectory(datalogger)
	list(APPEND PLUGINS ${DATALOGGER_TARGET_NAME})
endif()

if(ENABLE_PLUGIN_DAC)
	add_subdirectory(dac)
	list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

if(ENABLE_PLUGIN_SWIOT)
	add_subdirectory(swiot)
	list(APPEND PLUGINS ${SWIOT_TARGET_NAME})
endif()

if(ENABLE_PLUGIN_PQM)
	add_subdirectory(pqm)
	list(APPEND PLUGINS ${PQM_TARGET_NAME})
endif()


if(ENABLE_PLUGIN_M2K)
	if(NOT WITH_PYTHON)
		message(STATUS "Python is disabled or not found, M2K plugin disabled")
	endif()

	if(NOT WITH_SIGROK)
		message(STATUS "Libsigrokdecode is disabled or not found, M2K plugin disabled")
	endif()

	if(WITH_SIGROK AND WITH_PYTHON)
		add_subdirectory(m2k)
		list(APPEND PLUGINS ${M2K_TARGET_NAME})
	endif()
endif()

install(TARGETS ${PLUGINS} LIBRARY DESTINATION ${SCOPY_PLUGIN_INSTALL_PATH})

set(PLUGINS ${PLUGINS} PARENT_SCOPE)
set(PLUGIN_COMPONENTS ${PLUGIN_COMPONENTS} PARENT_SCOPE)
set(PLUGIN_COMPONENTS_FILES ${PLUGIN_COMPONENTS_FILES} PARENT_SCOPE)
