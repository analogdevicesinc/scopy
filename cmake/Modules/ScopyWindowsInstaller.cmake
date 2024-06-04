set(PLUGIN_COMPONENTS "" CACHE STRING "PLUGIN COMPONENTS FOR WINDOWS INSTALLER" FORCE)
set(PLUGIN_COMPONENTS_FILES "" CACHE STRING "PLUGIN COMPONENTS FILES FOR WINDOWS INSTALLER" FORCE)

function(ConfigureInstallerSettings PLUGIN_TARGET_NAME PLUGIN_INSTALLER_DESCRIPTION FIXED_INSTALLER_OPTION)
	set(SELECTABLE "; Flags: disablenouninstallwarning")
	if(${FIXED_INSTALLER_OPTION} MATCHES FALSE)
		set(SELECTABLE "compact custom ; Flags: fixed")
	endif()
	set(PLUGIN_COMPONENTS
	    "${PLUGIN_COMPONENTS} \n Name: \"plugins/${PLUGIN_TARGET_NAME}\"; Description: ${PLUGIN_INSTALLER_DESCRIPTION} ; Types: full plugins ${SELECTABLE} ;"
	    CACHE STRING "PLUGIN COMPONENTS FOR WINDOWS INSTALLER" FORCE
	)
	set(PLUGIN_COMPONENTS_FILES
	    "${PLUGIN_COMPONENTS_FILES} \n Source:\"{#DestFolder}\\plugins\\plugins\\libscopy-${PLUGIN_TARGET_NAME}.dll\"; DestDir: \"{app}\\plugins\\plugins\"; Components: plugins\\${PLUGIN_TARGET_NAME}; Flags: ignoreversion skipifsourcedoesntexist onlyifdoesntexist ;"
	    CACHE STRING "PLUGIN COMPONENTS FILES FOR WINDOWS INSTALLER" FORCE
	)
endfunction()
