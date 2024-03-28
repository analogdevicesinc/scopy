cmake_minimum_required(VERSION 3.9)
project(scopy-plugins VERSION 0.1 LANGUAGES CXX)

message(STATUS "Plugins folder: " ${SCOPY_PLUGIN_BUILD_PATH})
message(STATUS "Plugins folder after install: " ${SCOPY_PLUGIN_INSTALL_PATH})

option(ENABLE_PLUGIN_M2K "Enable m2k plugin" ON)
option(ENABLE_PLUGIN_TEST "Enable test plugin" OFF)
option(ENABLE_PLUGIN_SWIOT "Enable swiot plugin" ON)
option(ENABLE_PLUGIN_REGMAP "Enable regmap plugin" ON)
option(ENABLE_PLUGIN_DEBUGGER "Enable debugger plugin" ON)
option(ENABLE_PLUGIN_DATALOGGER "Enable datalogger plugin" OFF)
option(ENABLE_PLUGIN_DATAMONITOR "Enable datamonitor plugin" OFF)
option(ENABLE_PLUGIN_ADC "Enable ADC plugin" ON)
option(ENABLE_PLUGIN_PQM "Enable PQM plugin" ON)
option(ENABLE_PLUGIN_IIODEBUGPLUGIN "Enable IIODEBUGPLUGIN plugin" ON)

if(ENABLE_PLUGIN_TEST)
	add_subdirectory(guitestplugin)
	list(APPEND PLUGINS ${PLUGIN_NAME})
	add_subdirectory(testplugin)
	list(APPEND PLUGINS ${PLUGIN_NAME})
	add_subdirectory(testplugin2)
	list(APPEND PLUGINS ${PLUGIN_NAME})
	add_subdirectory(bareminimumplugin)
	list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

if(ENABLE_PLUGIN_ADC)
	add_subdirectory(adc)
	list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

if(ENABLE_PLUGIN_SWIOT)
	add_subdirectory(swiot)
	list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

if(ENABLE_PLUGIN_PQM)
	add_subdirectory(pqm)
	list(APPEND PLUGINS ${PLUGIN_NAME})
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
		list(APPEND PLUGINS ${PLUGIN_NAME})
	endif()
endif()

if(ENABLE_PLUGIN_REGMAP)
	add_subdirectory(regmap)
	list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

if(ENABLE_PLUGIN_DEBUGGER)
	add_subdirectory(debugger)
	list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

if(ENABLE_PLUGIN_DATALOGGER)
	add_subdirectory(datalogger)
	list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

if(ENABLE_PLUGIN_DATAMONITOR)
	add_subdirectory(datamonitor)
	list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

if(ENABLE_PLUGIN_IIODEBUGPLUGIN)
	add_subdirectory(iiodebugplugin)
	list(APPEND PLUGINS ${PLUGIN_NAME})
endif()

message(STATUS "Building plugins:" ${PLUGINS})

install(TARGETS ${PLUGINS} LIBRARY DESTINATION ${SCOPY_PLUGIN_INSTALL_PATH}
	RUNTIME DESTINATION ${SCOPY_PLUGIN_INSTALL_PATH}
)
