/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

evaluateFile("../js/scopycommon.js")

function switchToRegMap() {
    let toolSwitched = scopy.switchTool("Register map")
    if (!toolSwitched) {
        printToConsole("Failed to switch to Register Map")
        exit(1)
    }
}

function readRegister(reg) {
    let result = regmap.readRegister(reg)
    if (result.length === 0) {
        printToConsole("Failed to read register " + reg)
        exit(1)
    }
    return result
}

function switchDevice(devName) {
    let result = regmap.setDevice(devName)
    if (!result) {
        printToConsole("Failed to change device " + devName)
        exit(1)
    }
}