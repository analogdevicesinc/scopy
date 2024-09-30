evaluateFile("../js/scopycommon.js")

function switchToRegMap() {
    let toolSwitched = scopy.switchTool("Register map")
    if (!toolSwitched) {
        printToConsole("Failed to switch to Register Map")
        exit(1)
    }
}

function readRegister(reg) {
    let result = regmap.readRegister(reg)
    if (result.length === 0) {
        printToConsole("Failed to read register " + reg)
        exit(1)
    }
    return result
}

function switchDevice(devName) {
    let result = regmap.setDevice(devName)
    if (!result) {
        printToConsole("Failed to change device " + devName)
        exit(1)
    }
}