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

#ifndef MONITORSELECTIONMENU_HPP
#define MONITORSELECTIONMENU_HPP

#include "datamonitor/datamonitormodel.hpp"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QWidget>
#include <menucollapsesection.h>
#include <semiexclusivebuttongroup.h>

namespace scopy {
namespace datamonitor {
class MonitorSelectionMenu : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit MonitorSelectionMenu(QMap<QString, DataMonitorModel *> *monitorList, QButtonGroup *monitorsGroup,
				      QWidget *parent = nullptr);

	void addMonitor(DataMonitorModel *monitor);
	void removeDevice(QString device);

	QButtonGroup *monitorsGroup() const;

Q_SIGNALS:
	void requestMonitorToggled(bool toggled, QString monitorName);
	void monitorToggled(bool toggled, QString monitorName);
	void removeMonitor();
	void requestRemoveImportedDevice(QString device);
	void requestMonitorMenu(bool toggled, QString monitorName);

private:
	QWidget *deviceChannelsWidget;
	QWidget *importedChannelsWidget;
	QVBoxLayout *layout;
	QButtonGroup *m_monitorsGroup;
	QMap<QString, MenuCollapseSection *> deviceMap;

	void generateDeviceSection(QString device, bool import = false);
};
} // namespace datamonitor
} // namespace scopy
#endif // MONITORSELECTIONMENU_HPP
