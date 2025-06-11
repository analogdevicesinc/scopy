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

/* log data test */
function logData(tool, filePath, selectedMonitors) {
    datalogger.enableMonitorOfTool(tool, selectedMonitors[0])
    datalogger.enableMonitorOfTool(tool, selectedMonitors[1])
    datalogger.clearData()
    msleep(500)
    datalogger.setRunning(true)
    msleep(1000)
    datalogger.logAtPathForTool(tool, filePath)
    datalogger.continuousLogAtPathForTool(tool, filePath)
    msleep(5000)
    datalogger.disableMonitorOfTool(tool, selectedMonitors[0])
    datalogger.disableMonitorOfTool(tool, selectedMonitors[1])
    datalogger.setRunning(false)
    datalogger.stopContinuousLogForTool(tool)

     printToConsole("Data logged for tool : " + tool + " at :" + filePath)
}

/* load data test */
function loadData(tool, filePath, selectedMonitors) {

    datalogger.importDataFromPathForTool(tool, filePath)
    msleep(500)
    datalogger.enableMonitorOfTool(tool, "Import: test.csv:" + selectedMonitors[0])
    datalogger.enableMonitorOfTool(tool, "Import: test.csv:" + selectedMonitors[1])

    printToConsole("Data loaded for tool : " + tool + " from :" + filePath)
}

function getMonitors(tool){
    var monitors = datalogger.showAvailableMonitors()
    printToConsole("Monitors list: " + monitors)
    var monitorList = monitors.split(/\n/)
    var voltage0Monitor = monitorList[2]
    printToConsole("Monitor1 : " + voltage0Monitor)
    var voltage1Monitor = monitorList[3]
    printToConsole("Monitor2 : " + voltage1Monitor)
    var selectedMonitors = []
    selectedMonitors[0] = voltage0Monitor
    selectedMonitors[1] = voltage1Monitor

    return selectedMonitors
}

function createTool(deviceID){
    var newTool = datalogger.createTool()
    if (newTool !== "") {
        scopy.switchTool(deviceID, newTool)
        printToConsole("New tool created: " + newTool)
    } else {
        printToConsole("ERROR: Tool creation failed")
        exit(1)
    }
    return "DataLogger 1"
}

