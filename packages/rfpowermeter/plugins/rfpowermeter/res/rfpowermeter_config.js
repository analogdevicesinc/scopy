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

// RF Power Meter DataLogger Configuration Script
// This script configures the DataLogger plugin for RF Power Meter monitoring

function configureRFPowerMeter() {

    try {
        // Set tool name and display mode
        datalogger.setToolName("Data Logger ", "RF Power Meter");
        datalogger.setDisplayMode("RF Power Meter", 2);

        // Configure power5 (Power Forward)
        datalogger.setMonitorDisplayName("RF Power Meter", "powrms:power5", "Power Forward");
        datalogger.setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:power5", "");
        datalogger.setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:power5", "dBm");
        datalogger.enableMonitorOfTool("RF Power Meter", "powrms:power5");

        // Configure power6 (Power Reverse)
        datalogger.setMonitorDisplayName("RF Power Meter", "powrms:power6", "Power reverse");
        datalogger.setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:power6", "");
        datalogger.setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:power6", "dBm");
        datalogger.enableMonitorOfTool("RF Power Meter", "powrms:power6");

        // Configure voltage0 (Voltage in corrected)
        datalogger.setMonitorDisplayName("RF Power Meter", "powrms:voltage0", "Voltage in corrected");
        datalogger.setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:voltage0", "Voltage");
        datalogger.setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:voltage0", "mV");

        // Configure voltage1 (Voltage out corrected)
        datalogger.setMonitorDisplayName("RF Power Meter", "powrms:voltage1", "Voltage out corrected");
        datalogger.setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:voltage1", "Voltage");
        datalogger.setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:voltage1", "mV");

        // Configure voltage2 (Temperature)
        datalogger.setMonitorDisplayName("RF Power Meter", "powrms:voltage2", "Temperature");
        datalogger.setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:voltage2", "Voltage");
        datalogger.setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:voltage2", "mV");

        // Configure voltage3 (Voltage in)
        datalogger.setMonitorDisplayName("RF Power Meter", "powrms:voltage3", "Voltage in");
        datalogger.setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:voltage3", "Voltage");
        datalogger.setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:voltage3", "mV");

        // Configure voltage4 (Voltage out)
        datalogger.setMonitorDisplayName("RF Power Meter", "powrms:voltage4", "Voltage out");
        datalogger.setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:voltage4", "Voltage");
        datalogger.setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:voltage4", "mV");

        console.log("RF Power Meter configuration applied successfully");
        return true;

    } catch (error) {
        console.log("Error configuring RF Power Meter: " + error.toString());
        return false;
    }
}

// Execute the configuration
configureRFPowerMeter();