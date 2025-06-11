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
evaluateFile("../plugins/datalogger/js/dataLoggerFunctions.js")

function dataLogAndLoadTest(){
    const filePath = "../plugins/datalogger/js/test.csv"

    printToConsole("FILE IS " + filePath)

    //CONNECT TO EMU
    connect()

    msleep(1000)
    //TEST DATA LOGGER
    printToConsole(datalogger.showAvailableMonitors())

    //CREATE NEW TOOL
    msleep(1000)
    var tool = "DataLogger 0"

    if (tool !== "") {
        //GET 2 CHANNELS OF THE TOOL
        scopy.switchTool(tool)
        var selectedMonitors = getMonitors(tool)
        printToConsole("Monitors : " + selectedMonitors)
        //LOG DATA ON THE TOOL CHANNELS
        logData(tool, filePath, selectedMonitors)
        msleep(1000)
        //LOAD DATA FOR THE TOOL CHANNELS
        loadData(tool, filePath, selectedMonitors)
        msleep(1000)
    }
    disconnect()
}

dataLogAndLoadTest()

