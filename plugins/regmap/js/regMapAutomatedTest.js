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

//This import path is relative to where scopy is build
evaluateFile("../js/scopycommon.js")
evaluateFile("../plugins/regmap/js/regMapFunctions.js")


//Not working with EMU
function deviceTest() {
    connect()
    switchToRegMap()
    readRegister("0x00")
    regmap.write("0x00", "0x01")
    regmap.write("0x00", "0x00")
}

function deviceTestEMU(path) {
    connect()
    switchToRegMap()
    regmap.readInterval("0x00", "0x07")
    regmap.registerDump(path)
    regmap.writeFromFile(path)
}

function chageDevicesTest() {
    connect()
    switchToRegMap()
    switchDevice("cf-ad9361-dds-core-lpc")
    readRegister("0x00")
    regmap.write("0x00", "0x01")
}

//Not working with EMU
function writeRegisterTest() {
    connect()
    switchToRegMap()
    readRegister("0x00")
    regmap.write("0x00", "0x01")
    let readResult = readRegister("0x00")
    if (readResult != "0x01") {
        printToConsole("Failed to write")
        exit(1)
    }
}

function writeBitField(addr, val) {
    connect()
    switchToRegMap()
    regmap.writeBitField(addr, val)
}

deviceTest()
deviceTestEMU("")
chageDevicesTest()
writeRegisterTest()
writeBitField("0x01", "0x01")
disconnect()
scopy.exit()