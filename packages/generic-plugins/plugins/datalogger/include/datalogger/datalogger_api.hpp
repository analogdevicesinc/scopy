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

#ifndef DATALOGGER_API_H
#define DATALOGGER_API_H

#include "scopy-datalogger_export.h"
#include <pluginbase/apiobject.h>

namespace scopy::datamonitor {

class DataLoggerPlugin;

class SCOPY_DATALOGGER_EXPORT DataLogger_API : public ApiObject
{
	Q_OBJECT
public:
	explicit DataLogger_API(DataLoggerPlugin *dataLoggerPlugin);
	~DataLogger_API();

	// PLUGIN RELATED
	Q_INVOKABLE QString showAvailableMonitors();
	Q_INVOKABLE QString showAvailableDevices();
	Q_INVOKABLE QString showMonitorsOfDevice(QString device);
	Q_INVOKABLE QString enableMonitor(QString monitor);
	Q_INVOKABLE QString disableMonitor(QString monitor);
	Q_INVOKABLE void setRunning(bool running);
	Q_INVOKABLE void clearData();
	Q_INVOKABLE void changeTool(QString name);
	Q_INVOKABLE void setMinMax(bool enable);
	Q_INVOKABLE void changePrecision(int decimals);
	Q_INVOKABLE void setMinYAxis(double min);
	Q_INVOKABLE void setMaxYAxis(double max);

	// TOOL RELATED
	Q_INVOKABLE QString createTool();
	Q_INVOKABLE QString getToolList();
	Q_INVOKABLE QString enableMonitorOfTool(QString toolName, QString monitor);
	Q_INVOKABLE QString disableMonitorOfTool(QString toolName, QString monitor);
	Q_INVOKABLE void setLogPathOfTool(QString toolName, QString path);
	Q_INVOKABLE void logAtPathForTool(QString toolName, QString path);
	Q_INVOKABLE void continuousLogAtPathForTool(QString toolName, QString path);
	Q_INVOKABLE void stopContinuousLogForTool(QString toolName);
	Q_INVOKABLE void importDataFromPathForTool(QString toolName, QString path);
	Q_INVOKABLE void setMonitorDisplayName(QString toolName, QString monitor, QString displayName);
	Q_INVOKABLE void setMonitorUnitOfMeasurementName(QString toolName, QString monitor, QString name);
	Q_INVOKABLE void setMonitorUnitOfMeasurementSymbol(QString toolName, QString monitor, QString symbol);
	Q_INVOKABLE void setDisplayMode(QString toolName, int mode);
	Q_INVOKABLE void setToolName(QString currentToolName, QString newToolName);

private:
	DataLoggerPlugin *m_dataLoggerPlugin;
	QString m_activeTool;
};
} // namespace scopy::datamonitor
#endif // DATALOGGER_API_H
