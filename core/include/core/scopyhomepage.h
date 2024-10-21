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

#ifndef SCOPYHOMEPAGE_H
#define SCOPYHOMEPAGE_H

#include "device.h"
#include "pluginmanager.h"
#include "scopy-core_export.h"
#include "scopyhomeaddpage.h"

#include <QPushButton>
#include <QWidget>

namespace Ui {
class ScopyHomePage;
}

namespace scopy {
class SCOPY_CORE_EXPORT ScopyHomePage : public QWidget
{
	Q_OBJECT

public:
	explicit ScopyHomePage(QWidget *parent = nullptr, PluginManager *pm = nullptr);
	~ScopyHomePage();
	QCheckBox *scanControlBtn();
	QPushButton *scanBtn();
	void setScannerEnable(bool b);

Q_SIGNALS:
	void requestDevice(QString id);
	void deviceAddedToUi(QString id);
	void newDeviceAvailable(DeviceImpl *d);

	void displayNameChanged(QString id, QString newName);

public Q_SLOTS:
	void addDevice(QString id, Device *);
	void removeDevice(QString id);
	void viewDevice(QString id);
	void connectingDevice(QString);
	void connectDevice(QString);
	void disconnectDevice(QString);

private:
	Ui::ScopyHomePage *ui;
	ScopyHomeAddPage *add;
};
} // namespace scopy

#endif // SCOPYHOMEPAGE_H
