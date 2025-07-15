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

set(PACKAGE_COMPONENTS "" CACHE STRING "PACKAGE COMPONENTS FOR WINDOWS INSTALLER" FORCE)
set(PACKAGE_COMPONENTS_FILES "" CACHE STRING "PACKAGE COMPONENTS FILES FOR WINDOWS INSTALLER" FORCE)

function(ConfigureInstallerSettings PACKAGE_TARGET_NAME PACKAGE_INSTALLER_DESCRIPTION FIXED_INSTALLER_OPTION)

    if(${FIXED_INSTALLER_OPTION})
        set(SELECTABLE "compact custom ; Flags: fixed")
    else()
        set(SELECTABLE "; Flags: disablenouninstallwarning")
    endif()

	string(REPLACE "-" "_"  PACKAGE_TARGET_NAME_REPLACED "${PACKAGE_TARGET_NAME}")

	set(COMPONENT_ENTRY "Name: \"packages/${PACKAGE_TARGET_NAME_REPLACED}\"; Description: ${PACKAGE_INSTALLER_DESCRIPTION}; Types: full packages ${SELECTABLE};")
	if ("${PACKAGE_COMPONENTS}" STREQUAL "")
		set(PACKAGE_COMPONENTS "${COMPONENT_ENTRY}" CACHE STRING "PACKAGE COMPONENTS FOR WINDOWS INSTALLER" FORCE)
	else()
		set(PACKAGE_COMPONENTS "${PACKAGE_COMPONENTS}\n${COMPONENT_ENTRY}" CACHE STRING "PACKAGE COMPONENTS FOR WINDOWS INSTALLER" FORCE)
	endif()

	set(FILES_ENTRY "Source:\"{#DestFolder}\\packages\\${PACKAGE_TARGET_NAME}\\\*\"; DestDir: \"{app}\\packages\\${PACKAGE_TARGET_NAME}\"; Components: packages\\${PACKAGE_TARGET_NAME_REPLACED}; Flags: ignoreversion onlyifdoesntexist recursesubdirs createallsubdirs;")
    if ("${PACKAGE_COMPONENTS_FILES}" STREQUAL "")
        set(PACKAGE_COMPONENTS_FILES "${FILES_ENTRY}" CACHE STRING "PACKAGE COMPONENTS FILES FOR WINDOWS INSTALLER" FORCE)
    else()
        set(PACKAGE_COMPONENTS_FILES "${PACKAGE_COMPONENTS_FILES}\n${FILES_ENTRY}" CACHE STRING "PACKAGE COMPONENTS FILES FOR WINDOWS INSTALLER" FORCE)
    endif()

endfunction()
