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
