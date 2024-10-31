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

#ifndef DATAACQUISITIONMANAGER_HPP
#define DATAACQUISITIONMANAGER_HPP

#include <QElapsedTimer>
#include <QMap>
#include <QObject>

#include "datamonitor/datamonitormodel.hpp"
#include "scopy-datalogger_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGER_EXPORT DataAcquisitionManager : public QObject
{
	Q_OBJECT
public:
	explicit DataAcquisitionManager(QObject *parent = nullptr);

	void addMonitor(DataMonitorModel *monitor);
	void removeMonitor(QString monitorName);
	void removeDevice(QString device);
	void clearMonitorsData();

	// will read data once
	void readData();
	QList<QString> getActiveMonitors();
	void updateActiveMonitors(bool toggled, QString monitorName);
	QMap<QString, DataMonitorModel *> *getDataMonitorMap() const;

Q_SIGNALS:
	void requestUpdateActiveMonitors();
	void requestRead();
	void activeMonitorsUpdated();
	void monitorAdded(DataMonitorModel *monitor);
	void monitorRemoved(QString monitorName);
	void deviceRemoved(QString deviceName);

private:
	QMap<QString, int> *m_activeMonitorsMap;
	QMap<QString, DataMonitorModel *> *m_dataMonitorMap;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAACQUISITIONMANAGER_HPP
