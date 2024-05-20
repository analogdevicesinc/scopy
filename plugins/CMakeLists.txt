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
option(ENABLE_PLUGIN_IIODEBUGPLUGIN "Enable IIODEBUGPLUGIN plugin" ON)

set(PLUGIN_COMPONENTS "")
set(PLUGIN_COMPONENTS_FILES "")

function(ConfigureInstallerSettings PLUGIN_NAME PLUGIN_DESCRIPTION FIXED)
	string(REPLACE "scopy-" "" AUX_PLUGIN ${PLUGIN_NAME})

	set(PLUGIN_COMPONENTS
	    ${PLUGIN_COMPONENTS}
	    "\n Name: \"plugins/${AUX_PLUGIN}\"; Description: ${PLUGIN_DESCRIPTION} ; Types: full plugins; Flags: ${FIXED} "
	    PARENT_SCOPE
	)
	set(PLUGIN_COMPONENTS_FILES
	    ${PLUGIN_COMPONENTS_FILES}
	    "\n Source:\"{#DestFolder}\\plugins\\plugins\\libscopy-${AUX_PLUGIN}.dll\"; DestDir: \"{app}\\plugins\\plugins\"; Components: plugins\\${AUX_PLUGIN}; Flags: ignoreversion skipifsourcedoesntexist onlyifdoesntexist"
	    PARENT_SCOPE
	)
endfunction()

# The order the plugins are added here will also affect the installer order. Make sure fixed plugin are before optional
# ones.

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
	configureinstallersettings(${PLUGIN_NAME} ${PLUGIN_DESCRIPTION} "fixed")
endif()

if(ENABLE_PLUGIN_REGMAP)
	add_subdirectory(regmap)
	list(APPEND PLUGINS ${PLUGIN_NAME})
	configureinstallersettings(${PLUGIN_NAME} ${PLUGIN_DESCRIPTION} "fixed")
endif()

if(ENABLE_PLUGIN_DEBUGGER)
	add_subdirectory(debugger)
	list(APPEND PLUGINS ${PLUGIN_NAME})
	configureinstallersettings(${PLUGIN_NAME} ${PLUGIN_DESCRIPTION} "fixed")
endif()

if(ENABLE_PLUGIN_DATALOGGER)
	add_subdirectory(datalogger)
	list(APPEND PLUGINS ${PLUGIN_NAME})
	configureinstallersettings(${PLUGIN_NAME} ${PLUGIN_DESCRIPTION} "fixed")
endif()

if(ENABLE_PLUGIN_SWIOT)
	add_subdirectory(swiot)
	list(APPEND PLUGINS ${PLUGIN_NAME})
	configureinstallersettings(${PLUGIN_NAME} ${PLUGIN_DESCRIPTION} "disablenouninstallwarning")
endif()

if(ENABLE_PLUGIN_PQM)
	add_subdirectory(pqm)
	list(APPEND PLUGINS ${PLUGIN_NAME})
	configureinstallersettings(${PLUGIN_NAME} ${PLUGIN_DESCRIPTION} "disablenouninstallwarning")
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
		configureinstallersettings(${PLUGIN_NAME} ${PLUGIN_DESCRIPTION} "disablenouninstallwarning")
	endif()
endif()

message(STATUS "Building plugins:" ${PLUGINS})
message("PLUGIN_COMPONENTS" ${PLUGIN_COMPONENTS})
set(PLUGINS ${PLUGINS} PARENT_SCOPE)
set(PLUGIN_COMPONENTS ${PLUGIN_COMPONENTS} PARENT_SCOPE)
set(PLUGIN_COMPONENTS_FILES ${PLUGIN_COMPONENTS_FILES} PARENT_SCOPE)

install(TARGETS ${PLUGINS} LIBRARY DESTINATION ${SCOPY_PLUGIN_INSTALL_PATH}
	RUNTIME DESTINATION ${SCOPY_PLUGIN_INSTALL_PATH}
)
