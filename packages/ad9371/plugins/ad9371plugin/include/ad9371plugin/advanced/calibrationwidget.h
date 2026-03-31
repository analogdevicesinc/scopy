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

#ifndef CALIBRATIONWIDGET_H
#define CALIBRATIONWIDGET_H

#include "scopy-ad9371plugin_export.h"
#include <QWidget>
#include <QVBoxLayout>
#include <iio.h>
#include <menuonoffswitch.h>

namespace scopy {
class IIOWidgetGroup;
}

namespace scopy::ad9371 {

class SCOPY_AD9371PLUGIN_EXPORT CalibrationWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CalibrationWidget(iio_device *device, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~CalibrationWidget();

Q_SIGNALS:
	void readRequested();

private Q_SLOTS:
	void onCalibrationMaskChanged();
	void readCalibrationMaskFromDevice();

private:
	void setupUi();
	QWidget *createCalibrationMaskGroup(QWidget *parent);
	void writeCalibrationMaskToDevice();

	iio_device *m_device;
	IIOWidgetGroup *m_widgetGroup = nullptr;

	MenuOnOffSwitch *m_txQecCal;
	MenuOnOffSwitch *m_txLoLeakageCal;
	MenuOnOffSwitch *m_txLoLeakageExtCal;
	MenuOnOffSwitch *m_txBbFilterCal;
};

} // namespace scopy::ad9371

#endif // CALIBRATIONWIDGET_H
