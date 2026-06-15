/*
 * Reproducer for PlotMarkerController pure virtual crash.
 * Connects to PLUTO and exits — the crash happens during connectDevice
 * in ADC FFT plugin init, before this script gets control back.
 * Run in a loop until it crashes.
 */

evaluateFile("js/testAutomations/common/testFramework.js");

printToConsole("CRASH_REPRO: Attempting connection...");

var deviceId = scopy.addDevice("ip:192.168.2.1");
if (deviceId) {
    var connected = scopy.connectDevice(deviceId);
    printToConsole("CRASH_REPRO: connectDevice returned: " + connected);
    if (connected) {
        printToConsole("CRASH_REPRO: Connection succeeded — no crash this run");
        scopy.disconnectDevice(deviceId);
        scopy.removeDevice("ip:192.168.2.1");
    }
} else {
    printToConsole("CRASH_REPRO: addDevice failed");
}

printToConsole("CRASH_REPRO: CLEAN_EXIT");
scopy.exit();
