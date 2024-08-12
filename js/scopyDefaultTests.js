//This import path is relative to where scopy is build
evaluateFile("../js/scopycommon.js")

function connectionTest(){
    printToConsole("TEST CONNECTION")
    connect()
    disconnect()
}

function switchAllToolsTest() {
  printToConsole("TEST SWITCH ALL TOOLS")
  connect()
  let compatibleTools = scopy.getTools()
  for (let i = 0; i < compatibleTools.length; i++){
    let resultSwitch = scopy.switchTool(compatibleTools[i])
    if (!resultSwitch) {
      printToConsole("Switch all tools failed at " + compatibleTools[i])
    }
  }
  disconnect()
}

function switchToolsPluginTest(plugin) {
  printToConsole("TEST ALL TOOLS FROM PLUGIN")
  connect()
  let toolsFromPlugin = scopy.getToolsForPlugin(plugin)
  for (let i = 0; i < toolsFromPlugin.length; i++){
    let resultSwitch = scopy.switchTool(toolsFromPlugin[i])
    if (!resultSwitch) {
      printToConsole("Switch all tools for plugin failed at " + toolsFromPlugin[i])
    }
  }
  disconnect()
}

function runAllToolsTest() {
  printToConsole("TEST RUN ALL TOOLS")
  connect()
  let compatibleTools = scopy.getTools()
  for (let i = 0; i < compatibleTools.length; i++){
    let resultSwitch = scopy.switchTool(compatibleTools[i])
    if (!resultSwitch) {
      printToConsole("Test run all tools failed at " + compatibleTools[i])
    }
    if (scopy.getToolBtnState(compatibleTools[i]))
    {
      scopy.runTool(compatibleTools[i], true)
      scopy.runTool(compatibleTools[i], false)
    }
  }
  disconnect()
}

function runAllPluginToolsTest(plugin) {
  printToConsole("TEST RUN ALL TOOLS FOR PLUGIN")
  connect()
  let compatibleTools = scopy.getToolsForPlugin(plugin)
  for (let i = 0; i < compatibleTools.length; i++){
    let resultSwitch = scopy.switchTool(compatibleTools[i])
    if (!resultSwitch) {
      printToConsole("Test run all tools for plugin failed at " + compatibleTools[i])
    }
    if (scopy.getToolBtn(compatibleTools[i]))
    {
      scopy.runTool(compatibleTools[i], true)
      scopy.runTool(compatibleTools[i], false)
    }
  }
  disconnect()
}

function saveTest(fileName) {
  printToConsole("TEST SAVE")
  connect()
  let saveResult = scopy.saveSetup(fileName)
  if (!saveResult) {
    printToConsole("Save failed!")
  }
  disconnect()
}

function loadTest(fileName) {
  printToConsole("TEST LOAD")
  connect()
  let loadResult = scopy.loadSetup(fileName)
  if (!loadResult) {
    printToConsole("Load failed!")
  }
  disconnect()
}

connectionTest()
switchAllToolsTest()
switchToolsPluginTest("DataLoggerPlugin")
runAllToolsTest()
runAllPluginToolsTest("DataLoggerPlugin")
saveTest("testFile")
loadTest("testFile")
