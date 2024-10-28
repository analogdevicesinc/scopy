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
	    "${PLUGIN_COMPONENTS_FILES} \n Source:\"{#DestFolder}\\plugins\\libscopy-${PLUGIN_TARGET_NAME}.dll\"; DestDir: \"{app}\\plugins\"; Components: plugins\\${PLUGIN_TARGET_NAME}; Flags: ignoreversion skipifsourcedoesntexist onlyifdoesntexist ;"
	    CACHE STRING "PLUGIN COMPONENTS FILES FOR WINDOWS INSTALLER" FORCE
	)
endfunction()
