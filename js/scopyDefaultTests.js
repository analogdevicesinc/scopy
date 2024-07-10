//This import path is relative to where scopy is build
evaluateFile("../js/scpycommon.js")

function connectionTest(){
    printToConsole("TEST CONNECTION")
    connect()
    disconnect()
}

function switchToolTest(){
  printToConsole("TEST SWITCH TOOL")
    connect()
    scopy.switchTool("Time")
    scopy.switchTool("DataLogger 0")
    scopy.switchTool("Debugger")
    disconnect()
}

connectionTest()
switchToolTest()
