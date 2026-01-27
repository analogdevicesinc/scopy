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

#include "scopy-adrv9009plugin_export.h"
#include <QWidget>
#include <QPushButton>
#include <gui/widgets/menusectionwidget.h>
#include <iio.h>
#include <menuonoffswitch.h>

namespace scopy::adrv9009 {

class SCOPY_ADRV9009PLUGIN_EXPORT CalibrationWidget : public QWidget
{
	Q_OBJECT
public:
	CalibrationWidget(iio_device *device, QWidget *parent = nullptr);
	~CalibrationWidget();

Q_SIGNALS:
	void readRequested();

private slots:
	void onCalibrationMaskChanged();
	void readCalibrationMaskFromDevice();

private:
	iio_device *m_device;

	MenuOnOffSwitch *m_txLoLeakageCal;
	MenuOnOffSwitch *m_txQecCal;
	MenuOnOffSwitch *m_txLolExternalCal;
	MenuOnOffSwitch *m_rxQecCal;
	MenuOnOffSwitch *m_rxPhaseCorrectionCal;
	MenuOnOffSwitch *m_fhmCal;

	void setupUi();
	QWidget *createCalibrationMaskGroup(QWidget *parent);
	void writeCalibrationMaskToDevice();
};

} // namespace scopy::adrv9009

#endif // CALIBRATIONWIDGET_H
