//This import paht is relative to where scopy is build 
evaluateFile("../js/scpyDefaultJsFunctions.js")
evaluateFile("../plugins/datalogger/js/dataLoggerFunctions.js")

function dataLogAndLoadTest(){
    //TODO REPLACE WITH VAR
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

