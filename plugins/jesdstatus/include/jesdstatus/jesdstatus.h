/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef JESDSTATUS_H
#define JESDSTATUS_H

#include "scopy-jesdstatus_export.h"

#include <gui/widgets/menucombo.h>
#include <gui/tooltemplate.h>
#include <gui/widgets/toolbuttons.h>
#include <iio.h>
#include <QList>
#include <QWidget>
#include <QTimer>

namespace scopy::jesdstatus {
class SCOPY_JESDSTATUS_EXPORT JesdStatus : public QWidget
{
	Q_OBJECT
public:
	JesdStatus(QList<struct iio_device *> devLst, QWidget *parent = nullptr);
	~JesdStatus();

Q_SIGNALS:
	void running(bool toggled);

public Q_SLOTS:
	void runToggled(bool toggled);

private:
	ToolTemplate *tool;
	QTimer *m_timer;
	QList<struct iio_device *> m_deviceList;
	MenuComboWidget *m_deviceSelector;
	MapStackedWidget *jesdDeviceStack;

	void setupDevice(iio_device *dev);
	void poll();
};
} // namespace scopy::jesdstatus
#endif // JESDSTATUS_H
