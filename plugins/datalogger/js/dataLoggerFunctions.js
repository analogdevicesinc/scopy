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

