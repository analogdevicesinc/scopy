/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 */

// ad9361-regmap-demo package tests (regmap customized by another package)
//
// Requires the ad9361-regmap-demo package and iio-emu running the pluto XML:
//   iio-emu generic packages/generic-plugins/emu-xml/pluto.xml
//
//   TST.REGMAP_DEMO.DEVICE_PRESENT   - ad9361-phy is listed by regmap
//   TST.REGMAP_DEMO.PKG_XML          - register map comes from the package XML
//   TST.REGMAP_DEMO.FILE_READ        - register values are read from the values file
//   TST.REGMAP_DEMO.FILE_WRITE       - written values are stored in the values file and read back
//
// ============================================================

evaluateFile("../js/testAutomations/common/testFramework.js");

TestFramework.init("AD9361 Regmap Demo Package Tests");

var DEVICE = "ad9361-phy";

function readValue(addr) {
    regmap.readRegister(addr);
    msleep(200);
    return parseInt(regmap.getValueOfRegister(addr), 16);
}

if (!TestFramework.connectToDevice("ip:127.0.0.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

if (!switchToTool("Register map")) {
    printToConsole("ERROR: Cannot switch to Register map tool");
    scopy.exit();
}

TestFramework.runTest("TST.REGMAP_DEMO.DEVICE_PRESENT", function() {
    var devices = regmap.getAvailableDevicesName();
    printToConsole("  Available devices: " + devices);
    if (devices.indexOf(DEVICE) < 0) {
        return DEVICE + " not found";
    }
    if (!regmap.setDevice(DEVICE)) {
        return "Can't select " + DEVICE;
    }
    msleep(500);
    return true;
});

TestFramework.runTest("TST.REGMAP_DEMO.PKG_XML", function() {
    var info = regmap.getRegisterInfo("0x0");
    printToConsole("  0x0 info: " + info);
    if (info.join("|").indexOf("Description:[pkg]") < 0) {
        return "Register 0x0 is not described by the package XML";
    }
    // 0x020 exists in the generic ad9361-phy.xml but not in the 16 register demo subset
    var missing = regmap.getRegisterInfo("0x20");
    if (missing.length !== 0) {
        return "Register 0x20 found, generic XML is still used";
    }
    return true;
});

TestFramework.runTest("TST.REGMAP_DEMO.FILE_READ", function() {
    // values from resources/ad9361-regmap-demo/ad9361-phy-values.csv
    var value = readValue("0x5");
    printToConsole("  0x5 = 0x" + value.toString(16));
    return TestFramework.assertEqual(value, 0xaf, "0x5 should be read from the values file");
});

TestFramework.runTest("TST.REGMAP_DEMO.FILE_WRITE", function() {
    var original = readValue("0x1");
    regmap.write("0x1", "0x33");
    msleep(300);
    // write strategy stores the value in the file, readback goes through the file read strategy
    var readback = readValue("0x1");
    printToConsole("  0x1 original 0x" + original.toString(16) + ", readback 0x" + readback.toString(16));
    regmap.write("0x1", "0x" + original.toString(16));
    msleep(300);
    return TestFramework.assertEqual(readback, 0x33, "0x1 readback after write");
});

TestFramework.disconnectFromDevice();
TestFramework.printSummary();
scopy.exit();
