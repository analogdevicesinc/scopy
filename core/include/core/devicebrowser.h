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

#ifndef DEVICEBROWSER_H
#define DEVICEBROWSER_H

#include "scopy-core_export.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QMap>
#include <QWidget>

#include <deviceicon.h>

namespace Ui {
class DeviceBrowser;
}

namespace scopy {
class SCOPY_CORE_EXPORT DeviceBrowser : public QWidget
{
	Q_OBJECT

public:
	explicit DeviceBrowser(QWidget *parent = nullptr);
	~DeviceBrowser();
	QAbstractButton *getDeviceWidgetFor(QString id);
	void addDevice(QString id, Device *d, int position = -1);
	void removeDevice(QString id);
	void connectDevice(QString id);
	void disconnectDevice(QString id);

Q_SIGNALS:
	void requestDevice(QString id, int direction);
	void requestRemoveDevice(QString id);
	void displayNameChanged(QString id, QString newName);

public Q_SLOTS:

	void nextDevice();
	void prevDevice();

private Q_SLOTS:
	void updateSelectedDeviceIdx(QString);
	void forwardRequestDeviceWithDirection();

private:
	void initBtns();
	DeviceIcon *buildDeviceIcon(Device *d, QWidget *parent = nullptr);
	Ui::DeviceBrowser *ui;
	QButtonGroup *bg;
	QHBoxLayout *layout;
	QList<QAbstractButton *> list;
	int currentIdx;

	int getIndexOfId(QString k);
	QString getIdOfIndex(int idx);
	const char *devBrowserId = "DeviceBrowserId";
};
} // namespace scopy
#endif // DEVICEBROWSER_H
