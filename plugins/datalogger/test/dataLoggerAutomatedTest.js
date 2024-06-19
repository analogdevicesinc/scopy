
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

/* main function */
function main(){
    //TODO REPLACE WITH VAR
    const filePath = "/home/ubuntu/Documents/git/scopy/dev/plugins/datalogger/test/automatedTests" + "/test.csv"

    printToConsole("FILE IS " + filePath)
    //CONNECT TO EMU
    var deviceID = scopy.addDevice("", "ip:127.0.0.0")
    scopy.connectDevice(deviceID)

    //TEST DATA LOGGER
    printToConsole(datalogger.showAvailableMonitors())

    //CREATE NEW TOOL
    var tool = createTool(deviceID)

     printToConsole("FILE IS " + tool)

    if (tool !== "") {
        //GET 2 CHANNELS OF THE TOOL
        var selectedMonitors = getMonitors(tool)
        printToConsole("Monitors : " + selectedMonitors)
        //LOG DATA ON THE TOOL CHANNELS
        logData(tool, filePath, selectedMonitors)
        msleep(2000)
        //LOAD DATA FOR THE TOOL CHANNELS
        loadData(tool, filePath, selectedMonitors)
        msleep(2000)
    }

}

main()
