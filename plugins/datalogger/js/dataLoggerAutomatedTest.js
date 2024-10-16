//This import path is relative to where scopy is build
evaluateFile("../js/scopycommon.js")
evaluateFile("../plugins/datalogger/js/dataLoggerFunctions.js")

//Plot
function readDataTest() {
    connect()
    scopy.switchTool("DataLogger 0")
    datalogger.enableMonitorOfTool("DataLogger 0","xadc:temp0")
    datalogger.setRunning(true)
    msleep(5000)
    datalogger.setRunning(false)
    disconnect()
}

function readClearDataTest() {
    connect()
    scopy.switchTool("DataLogger 0")
    datalogger.enableMonitorOfTool("DataLogger 0","xadc:temp0")
    datalogger.setRunning(true)
    msleep(3000)
    datalogger.setRunning(false)
    datalogger.clearData()
    disconnect()
}

function readPrintDataTest() {
    connect()
    scopy.switchTool("DataLogger 0")
    datalogger.enableMonitorOfTool("DataLogger 0","xadc:temp0")
    datalogger.setRunning(true)
    msleep(3000)
    datalogger.setRunning(false)
    datalogger.print("../plugins/datalogger/js")
    disconnect()
}

//7 Segment
function readDataSegmentTest() {
    connect()
    scopy.switchTool("DataLogger 0")
    datalogger.enableMonitorOfTool("DataLogger 0","xadc:temp0")
    datalogger.changeTool("7 Segment")
    datalogger.setRunning(true)
    msleep(2000)
    datalogger.setRunning(false)
    disconnect()
}

function minMaxSegementText() {
    connect()
    scopy.switchTool("DataLogger 0")
    datalogger.enableMonitorOfTool("DataLogger 0","xadc:temp0")
    datalogger.changeTool("7 Segment")
    datalogger.setMinMax(false)
    datalogger.setRunning(true)
    msleep(2000)
    datalogger.setRunning(false)
    disconnect()
}

function changeSegmentPrecisionTest() {
    connect()
    scopy.switchTool("DataLogger 0")
    datalogger.enableMonitorOfTool("DataLogger 0","xadc:temp0")
    datalogger.changeTool("7 Segment")
    datalogger.changePrecision(2)
    datalogger.setRunning(true)
    msleep(2000)
    datalogger.setRunning(false)
    disconnect()
}

//Text
function readDataTextTest() {
    connect()
    scopy.switchTool("DataLogger 0")
    datalogger.enableMonitorOfTool("DataLogger 0","xadc:temp0")
    datalogger.changeTool("Text")
    datalogger.setRunning(true)
    msleep(2000)
    datalogger.setRunning(false)
    disconnect()
}

function readMultipleTextTest() {
    connect()
    scopy.switchTool("DataLogger 0")
    datalogger.enableMonitorOfTool("DataLogger 0", "xadc:temp0")
    datalogger.enableMonitorOfTool("DataLogger 0", "xadc:voltage0")
    datalogger.changeTool("Text")
    datalogger.setRunning(true)
    msleep(2000)
    datalogger.disableMonitorOfTool("DataLogger 0", "xadc:voltage0")
    msleep(2000)
    datalogger.setRunning(false)
    disconnect()
}

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

